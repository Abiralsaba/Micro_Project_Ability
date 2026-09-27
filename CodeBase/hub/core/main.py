#!/usr/bin/env python3
"""
Project Ability — Central Hub Engine
Runs on Raspberry Pi 5

This is the brain of the ecosystem. It:
1. Subscribes to all module MQTT topics
2. Routes messages between modules
3. Converts text → braille commands
4. Converts text → TTS audio (future)

Usage:
    python3 main.py
"""

import json
import time
import logging
import paho.mqtt.client as mqtt

# ═══════════════════════════════════════════════════════════
# CONFIGURATION
# ═══════════════════════════════════════════════════════════

MQTT_HOST = "localhost"
MQTT_PORT = 1883
HUB_ID = "hub_001"

# Topics to subscribe to (incoming from modules)
SUBSCRIBE_TOPICS = [
    "ability/glove/text",       # Glove sends recognized text
    "ability/voice/text",       # Voice module sends transcribed text
    "ability/gaze/text",        # Gaze module sends selected text
    "ability/braille/status",   # Braille module status updates
    "ability/glove/status",     # Glove module status updates
    "ability/hub/command",      # Commands from any source (web UI, etc)
]

# Topics to publish to (outgoing to modules)
TOPIC_BRAILLE_DISPLAY = "ability/braille/display"
TOPIC_VOICE_SPEAK = "ability/voice/speak"
TOPIC_HUB_STATUS = "ability/hub/status"

# Registered modules
modules_online = {}

# ═══════════════════════════════════════════════════════════
# LOGGING
# ═══════════════════════════════════════════════════════════

logging.basicConfig(
    level=logging.INFO,
    format="%(asctime)s [%(levelname)s] %(message)s",
    datefmt="%H:%M:%S",
)
log = logging.getLogger("hub")

# ═══════════════════════════════════════════════════════════
# TEXT → BRAILLE CONVERTER
# ═══════════════════════════════════════════════════════════
# This is the same braille encoding used in the ESP32 firmware.
# The Hub sends plain text to the Braille ESP32, and the ESP32
# handles the actual servo driving. But we keep the lookup here
# too for validation and logging.

BRAILLE_MAP = {
    'a': 0b000001, 'b': 0b000011, 'c': 0b001001, 'd': 0b011001,
    'e': 0b010001, 'f': 0b001011, 'g': 0b011011, 'h': 0b010011,
    'i': 0b001010, 'j': 0b011010, 'k': 0b000101, 'l': 0b000111,
    'm': 0b001101, 'n': 0b011101, 'o': 0b010101, 'p': 0b001111,
    'q': 0b011111, 'r': 0b010111, 's': 0b001110, 't': 0b011110,
    'u': 0b100101, 'v': 0b100111, 'w': 0b111010, 'x': 0b101101,
    'y': 0b111101, 'z': 0b110101, ' ': 0b000000,
}

def text_to_braille_dots(text):
    """Convert text to list of 6-bit braille patterns."""
    patterns = []
    for ch in text.lower():
        if ch in BRAILLE_MAP:
            patterns.append(BRAILLE_MAP[ch])
        else:
            patterns.append(None)  # unsupported char
    return patterns


# ═══════════════════════════════════════════════════════════
# MQTT CALLBACKS
# ═══════════════════════════════════════════════════════════

def on_connect(client, userdata, flags, rc):
    if rc == 0:
        log.info("Connected to MQTT broker at %s:%d", MQTT_HOST, MQTT_PORT)
        for topic in SUBSCRIBE_TOPICS:
            client.subscribe(topic)
            log.info("  Subscribed: %s", topic)

        # Announce hub is online
        client.publish(TOPIC_HUB_STATUS, json.dumps({
            "online": True,
            "hub_id": HUB_ID,
            "timestamp": time.time(),
        }))
    else:
        log.error("MQTT connection failed with code %d", rc)


def on_message(client, userdata, msg):
    topic = msg.topic
    try:
        payload = msg.payload.decode("utf-8")
    except:
        payload = str(msg.payload)

    log.info("◄ Received [%s]: %s", topic, payload[:200])

    # Try to parse as JSON
    data = None
    try:
        data = json.loads(payload)
    except json.JSONDecodeError:
        # Plain text payload — wrap it
        data = {"text": payload}

    # ─── ROUTE: Glove → Braille ──────────────────────────
    if topic == "ability/glove/text":
        text = data.get("text", "")
        if text:
            route_to_braille(client, text, source="glove")
            route_to_voice(client, text, source="glove")

    # ─── ROUTE: Voice → Braille ──────────────────────────
    elif topic == "ability/voice/text":
        text = data.get("text", "")
        if text:
            route_to_braille(client, text, source="voice")

    # ─── ROUTE: Gaze → Braille + Voice ──────────────────
    elif topic == "ability/gaze/text":
        text = data.get("text", "")
        if text:
            route_to_braille(client, text, source="gaze")
            route_to_voice(client, text, source="gaze")

    # ─── Hub Commands ───────────────────────────────────
    elif topic == "ability/hub/command":
        handle_command(client, data)

    # ─── Status Updates ─────────────────────────────────
    elif topic.endswith("/status"):
        module_name = topic.split("/")[1]
        modules_online[module_name] = {
            "data": data,
            "last_seen": time.time(),
        }
        log.info("  Module '%s' status updated", module_name)


# ═══════════════════════════════════════════════════════════
# ROUTING FUNCTIONS
# ═══════════════════════════════════════════════════════════

def route_to_braille(client, text, source="unknown"):
    """Send text to the Braille module for display."""
    log.info("► Routing to Braille: \"%s\" (from %s)", text, source)

    # Validate braille conversion
    patterns = text_to_braille_dots(text)
    supported = sum(1 for p in patterns if p is not None)
    log.info("  Braille: %d/%d chars supported", supported, len(patterns))

    payload = json.dumps({
        "text": text,
        "from": source,
        "timestamp": time.time(),
    })

    client.publish(TOPIC_BRAILLE_DISPLAY, payload)
    log.info("  Published to %s", TOPIC_BRAILLE_DISPLAY)


def route_to_voice(client, text, source="unknown"):
    """Send text to the Voice module for TTS playback."""
    log.info("► Routing to Voice: \"%s\" (from %s)", text, source)

    payload = json.dumps({
        "text": text,
        "from": source,
        "timestamp": time.time(),
    })

    client.publish(TOPIC_VOICE_SPEAK, payload)
    log.info("  Published to %s", TOPIC_VOICE_SPEAK)


def handle_command(client, data):
    """Handle commands sent to the hub."""
    cmd = data.get("cmd", "")
    target = data.get("target", "all")

    log.info("► Command: '%s' target='%s'", cmd, target)

    if cmd == "home" and target in ("braille", "all"):
        client.publish(TOPIC_BRAILLE_DISPLAY, json.dumps({"text": "home"}))
    elif cmd == "test" and target in ("braille", "all"):
        client.publish(TOPIC_BRAILLE_DISPLAY, json.dumps({"text": "test"}))
    elif cmd == "status":
        log.info("  Online modules: %s", list(modules_online.keys()))


def on_disconnect(client, userdata, rc):
    log.warning("Disconnected from MQTT broker (rc=%d). Reconnecting...", rc)


# ═══════════════════════════════════════════════════════════
# MAIN
# ═══════════════════════════════════════════════════════════

def main():
    log.info("═" * 50)
    log.info("  Project Ability — Central Hub Engine")
    log.info("  MQTT: %s:%d", MQTT_HOST, MQTT_PORT)
    log.info("═" * 50)

    client = mqtt.Client(client_id=HUB_ID)
    client.on_connect = on_connect
    client.on_message = on_message
    client.on_disconnect = on_disconnect

    # Auto-reconnect
    client.reconnect_delay_set(min_delay=1, max_delay=30)

    try:
        client.connect(MQTT_HOST, MQTT_PORT, keepalive=60)
    except ConnectionRefusedError:
        log.error("Cannot connect to MQTT broker at %s:%d", MQTT_HOST, MQTT_PORT)
        log.error("Is Mosquitto running? Try: sudo systemctl start mosquitto")
        return

    log.info("Hub engine running. Press Ctrl+C to stop.")
    log.info("")

    try:
        client.loop_forever()
    except KeyboardInterrupt:
        log.info("")
        log.info("Shutting down hub...")
        client.publish(TOPIC_HUB_STATUS, json.dumps({
            "online": False,
            "hub_id": HUB_ID,
            "timestamp": time.time(),
        }))
        client.disconnect()
        log.info("Hub stopped.")


if __name__ == "__main__":
    main()
