"""Secure MQTT transport for the Ability gaze keyboard."""

from __future__ import annotations

import json
import ssl
import threading
import time
import uuid
from dataclasses import dataclass
from typing import Optional

import paho.mqtt.client as mqtt

from hub.chat_config import (
    MQTT_HOST,
    MQTT_PASSWORD,
    MQTT_PORT,
    MQTT_USERNAME,
    MQTT_USE_TLS,
    TOPIC_INPUT,
    TOPIC_OUTPUT,
    TOPIC_STATUS,
)


@dataclass(frozen=True)
class IncomingMessage:
    source: str
    text: str
    timestamp: float


class AbilityGazeCloud:
    """Publishes gaze text and receives messages for ``gaze_01``."""

    def __init__(self, device_id: str = "gaze_01") -> None:
        self.device_id = device_id
        self.input_topic = TOPIC_INPUT.format(device=device_id)
        self.output_topic = TOPIC_OUTPUT.format(device=device_id)
        self.status_topic = TOPIC_STATUS.format(device=device_id)
        self.connected = threading.Event()
        self._lock = threading.Lock()
        self._incoming: Optional[IncomingMessage] = None
        self._calibrated = False
        self._target = "glove_01"

        self.client = mqtt.Client(
            mqtt.CallbackAPIVersion.VERSION2,
            client_id=f"{device_id}-{uuid.uuid4().hex[:8]}",
            protocol=mqtt.MQTTv5,
        )
        if MQTT_USERNAME:
            self.client.username_pw_set(MQTT_USERNAME, MQTT_PASSWORD)
        if MQTT_USE_TLS:
            self.client.tls_set(
                cert_reqs=ssl.CERT_REQUIRED,
                tls_version=ssl.PROTOCOL_TLS_CLIENT,
            )

        offline = json.dumps({
            "module": device_id,
            "device_type": "gaze",
            "online": False,
        })
        self.client.will_set(self.status_topic, offline, qos=1, retain=True)
        self.client.on_connect = self._on_connect
        self.client.on_disconnect = self._on_disconnect
        self.client.on_message = self._on_message
        self.client.reconnect_delay_set(min_delay=1, max_delay=30)

    def _on_connect(self, client, userdata, flags, reason_code, properties) -> None:
        if reason_code.is_failure:
            return
        self.connected.set()
        client.subscribe(self.output_topic, qos=1)
        self.publish_presence(self._calibrated, self._target)

    def _on_disconnect(
        self, client, userdata, disconnect_flags, reason_code, properties
    ) -> None:
        self.connected.clear()

    def _on_message(self, client, userdata, message) -> None:
        if message.topic != self.output_topic:
            return
        try:
            data = json.loads(message.payload.decode("utf-8"))
        except (UnicodeDecodeError, json.JSONDecodeError):
            return
        text = data.get("text")
        if not isinstance(text, str) or not text.strip():
            return
        incoming = IncomingMessage(
            source=str(data.get("source", "Ability Chat")),
            text=text.strip(),
            timestamp=time.time(),
        )
        with self._lock:
            self._incoming = incoming

    def start(self) -> None:
        self.client.connect_async(MQTT_HOST, MQTT_PORT, keepalive=60)
        self.client.loop_start()

    def publish_presence(self, calibrated: bool, target: str) -> bool:
        self._calibrated = calibrated
        self._target = target
        if not self.connected.is_set():
            return False
        payload = json.dumps({
            "module": self.device_id,
            "device_type": "gaze",
            "profile": "parkinsons",
            "interface": "eye_keyboard",
            "online": True,
            "calibrated": calibrated,
            "target": target,
            "timestamp": int(time.time()),
        })
        result = self.client.publish(self.status_topic, payload, qos=1, retain=True)
        return result.rc == mqtt.MQTT_ERR_SUCCESS

    def publish_text(self, text: str, target: str) -> bool:
        clean_text = text.strip()
        if not clean_text or not self.connected.is_set():
            return False
        payload = json.dumps({
            "version": 1,
            "message_id": str(uuid.uuid4()),
            "source": self.device_id,
            "target": target,
            "type": "text",
            "text": clean_text,
            "profile": "parkinsons",
            "timestamp": int(time.time()),
        })
        result = self.client.publish(self.input_topic, payload, qos=1)
        return result.rc == mqtt.MQTT_ERR_SUCCESS

    def latest_incoming(self) -> Optional[IncomingMessage]:
        with self._lock:
            return self._incoming

    def close(self) -> None:
        if self.connected.is_set():
            payload = json.dumps({
                "module": self.device_id,
                "device_type": "gaze",
                "online": False,
                "timestamp": int(time.time()),
            })
            info = self.client.publish(self.status_topic, payload, qos=1, retain=True)
            try:
                info.wait_for_publish(timeout=2.0)
            except RuntimeError:
                pass
        self.client.disconnect()
        self.client.loop_stop()
