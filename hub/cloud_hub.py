#!/usr/bin/env python3
"""Project Ability cloud MQTT hub.

The Raspberry Pi subscribes to every module input topic, performs the
device-independent routing, and publishes only device-ready output messages.
EMQX is used only as an Internet transport.
"""

from __future__ import annotations

import argparse
import json
import logging
import os
import threading
import time
import uuid
from dataclasses import dataclass
from pathlib import Path

import paho.mqtt.client as mqtt


CODEBASE_DIR = Path(__file__).resolve().parents[2]
ENV_FILE = CODEBASE_DIR / ".env"
INPUT_TOPIC = "ability/v1/+/input"
KNOWN_TARGETS = {"glove_01", "braille_01", "voice_01", "gaze_01"}

BRAILLE_MAP = {
    "a": 0b000001, "b": 0b000011, "c": 0b001001, "d": 0b011001,
    "e": 0b010001, "f": 0b001011, "g": 0b011011, "h": 0b010011,
    "i": 0b001010, "j": 0b011010, "k": 0b000101, "l": 0b000111,
    "m": 0b001101, "n": 0b011101, "o": 0b010101, "p": 0b001111,
    "q": 0b011111, "r": 0b010111, "s": 0b001110, "t": 0b011110,
    "u": 0b100101, "v": 0b100111, "w": 0b111010, "x": 0b101101,
    "y": 0b111101, "z": 0b110101, " ": 0b000000,
}

logging.basicConfig(
    level=logging.INFO,
    format="%(asctime)s [%(levelname)s] %(message)s",
    datefmt="%Y-%m-%d %H:%M:%S",
)
log = logging.getLogger("ability-cloud-hub")


def load_env_file(path: Path) -> None:
    """Load a small KEY=VALUE file without adding another dependency."""
    if not path.exists():
        return
    for raw_line in path.read_text(encoding="utf-8").splitlines():
        line = raw_line.strip()
        if not line or line.startswith("#") or "=" not in line:
            continue
        key, value = line.split("=", 1)
        os.environ.setdefault(key.strip(), value.strip().strip("'\""))


@dataclass(frozen=True)
class Settings:
    host: str
    port: int
    username: str
    password: str
    ca_file: Path
    hub_id: str

    @classmethod
    def from_environment(cls) -> "Settings":
        load_env_file(ENV_FILE)
        required = {
            "ABILITY_MQTT_HOST": os.getenv("ABILITY_MQTT_HOST"),
            "ABILITY_MQTT_USERNAME": os.getenv("ABILITY_MQTT_USERNAME"),
            "ABILITY_MQTT_PASSWORD": os.getenv("ABILITY_MQTT_PASSWORD"),
            "ABILITY_MQTT_CA": os.getenv("ABILITY_MQTT_CA"),
        }
        missing = [key for key, value in required.items() if not value]
        if missing:
            raise RuntimeError(f"Missing settings in {ENV_FILE}: {', '.join(missing)}")

        ca_file = Path(required["ABILITY_MQTT_CA"])
        if not ca_file.is_absolute():
            ca_file = CODEBASE_DIR / ca_file
        if not ca_file.is_file():
            raise RuntimeError(f"CA certificate not found: {ca_file}")

        return cls(
            host=required["ABILITY_MQTT_HOST"],
            port=int(os.getenv("ABILITY_MQTT_PORT", "8883")),
            username=required["ABILITY_MQTT_USERNAME"],
            password=required["ABILITY_MQTT_PASSWORD"],
            ca_file=ca_file,
            hub_id=os.getenv("ABILITY_HUB_ID", "hub_001"),
        )


def text_to_braille(text: str) -> list[int]:
    """Return Grade-1 six-dot patterns; unsupported characters become blank."""
    return [BRAILLE_MAP.get(character.lower(), 0) for character in text]


class AbilityCloudHub:
    def __init__(self, settings: Settings, self_test: bool = False) -> None:
        self.settings = settings
        self.self_test = self_test
        self.connected = threading.Event()
        self.test_received = threading.Event()
        self.test_token = uuid.uuid4().hex
        self.test_topic = f"ability/v1/hubs/{settings.hub_id}/selftest"

        client_id = f"{settings.hub_id}-check" if self_test else settings.hub_id
        self.client = mqtt.Client(
            mqtt.CallbackAPIVersion.VERSION2,
            client_id=client_id,
            protocol=mqtt.MQTTv5,
        )
        self.client.username_pw_set(settings.username, settings.password)
        self.client.tls_set(ca_certs=str(settings.ca_file))
        self.client.on_connect = self._on_connect
        self.client.on_disconnect = self._on_disconnect
        self.client.on_message = self._on_message
        self.client.reconnect_delay_set(min_delay=1, max_delay=30)

        status_topic = f"ability/v1/hubs/{settings.hub_id}/status"
        offline = json.dumps({"online": False, "hub_id": settings.hub_id})
        self.client.will_set(status_topic, offline, qos=1, retain=True)

    def _on_connect(self, client, userdata, flags, reason_code, properties) -> None:
        if reason_code.is_failure:
            log.error("MQTT connection rejected: %s", reason_code)
            return

        log.info("Connected securely to %s:%d", self.settings.host, self.settings.port)
        client.subscribe(INPUT_TOPIC, qos=1)
        log.info("Subscribed to %s", INPUT_TOPIC)

        status_topic = f"ability/v1/hubs/{self.settings.hub_id}/status"
        client.publish(
            status_topic,
            json.dumps({
                "online": True,
                "hub_id": self.settings.hub_id,
                "timestamp": int(time.time()),
            }),
            qos=1,
            retain=True,
        )
        self.connected.set()

        if self.self_test:
            client.subscribe(self.test_topic, qos=1)
            client.publish(self.test_topic, self.test_token, qos=1)

    def _on_disconnect(self, client, userdata, disconnect_flags, reason_code, properties) -> None:
        if reason_code.is_failure:
            log.warning("MQTT disconnected unexpectedly: %s", reason_code)

    def _on_message(self, client, userdata, message) -> None:
        if message.topic == self.test_topic:
            if message.payload.decode("utf-8", errors="replace") == self.test_token:
                self.test_received.set()
            return

        try:
            data = json.loads(message.payload)
        except (json.JSONDecodeError, UnicodeDecodeError):
            log.warning("Ignoring invalid JSON from %s", message.topic)
            return

        parts = message.topic.split("/")
        if len(parts) != 4:
            log.warning("Ignoring unexpected topic: %s", message.topic)
            return

        source = parts[2]
        target = data.get("target")
        text = data.get("text")

        if target not in KNOWN_TARGETS:
            log.warning("Message from %s has an unknown target: %r", source, target)
            return
        if not isinstance(text, str) or not text.strip():
            log.warning("Message from %s has no processed text yet", source)
            return

        message_id = data.get("message_id") or str(uuid.uuid4())
        output = {
            "version": 1,
            "message_id": message_id,
            "source": source,
            "target": target,
            "timestamp": int(time.time()),
        }

        if target == "braille_01":
            output.update({"type": "braille", "braille": text_to_braille(text)})
        else:
            output.update({"type": "text", "text": text})

        output_topic = f"ability/v1/{target}/output"
        result = client.publish(output_topic, json.dumps(output), qos=1)
        if result.rc == mqtt.MQTT_ERR_SUCCESS:
            log.info("Routed %s -> %s (%s)", source, target, message_id)
        else:
            log.error("Publish to %s failed with code %s", output_topic, result.rc)

    def check_connection(self, timeout: float = 15.0) -> bool:
        self.client.connect(self.settings.host, self.settings.port, keepalive=60)
        self.client.loop_start()
        try:
            if not self.connected.wait(timeout):
                log.error("Timed out connecting to the broker")
                return False
            if not self.test_received.wait(timeout):
                log.error("Connected, but the publish/subscribe self-test timed out")
                return False
            log.info("TLS publish/subscribe self-test passed")
            return True
        finally:
            self.client.disconnect()
            self.client.loop_stop()

    def run_forever(self) -> None:
        log.info("Starting Ability hub %s", self.settings.hub_id)
        self.client.connect(self.settings.host, self.settings.port, keepalive=60)
        self.client.loop_forever()


def main() -> int:
    parser = argparse.ArgumentParser(description="Project Ability cloud MQTT hub")
    parser.add_argument(
        "--check",
        action="store_true",
        help="verify TLS connection and MQTT round-trip, then exit",
    )
    args = parser.parse_args()

    try:
        settings = Settings.from_environment()
        hub = AbilityCloudHub(settings, self_test=args.check)
        if args.check:
            return 0 if hub.check_connection() else 1
        hub.run_forever()
        return 0
    except (OSError, RuntimeError, ValueError) as error:
        log.error("Hub startup failed: %s", error)
        return 1
    except KeyboardInterrupt:
        log.info("Hub stopped")
        return 0


if __name__ == "__main__":
    raise SystemExit(main())
