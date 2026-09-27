#!/usr/bin/env python3
"""Send a text message through the live Ability cloud hub."""

import argparse
import json
import sys
import time
import uuid
from pathlib import Path

import paho.mqtt.client as mqtt

CODEBASE_DIR = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(CODEBASE_DIR))

from hub.core.cloud_hub import Settings  # noqa: E402


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--text", required=True)
    parser.add_argument(
        "--target",
        choices=("glove_01", "braille_01", "voice_01", "gaze_01"),
        default="braille_01",
    )
    parser.add_argument("--source", default="manual_test")
    args = parser.parse_args()

    settings = Settings.from_environment()
    client = mqtt.Client(
        mqtt.CallbackAPIVersion.VERSION2,
        client_id=f"manual-send-{uuid.uuid4().hex[:8]}",
        protocol=mqtt.MQTTv5,
    )
    client.username_pw_set(settings.username, settings.password)
    client.tls_set(ca_certs=str(settings.ca_file))
    client.connect(settings.host, settings.port, keepalive=30)
    client.loop_start()
    topic = f"ability/v1/{args.source}/input"
    payload = json.dumps({
        "message_id": str(uuid.uuid4()),
        "target": args.target,
        "text": args.text,
        "timestamp": int(time.time()),
    })
    result = client.publish(topic, payload, qos=1)
    result.wait_for_publish(timeout=10)
    client.disconnect()
    client.loop_stop()
    if result.rc != mqtt.MQTT_ERR_SUCCESS:
        print(f"Publish failed with MQTT code {result.rc}")
        return 1
    print(f"Sent to {args.target}: {args.text}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
