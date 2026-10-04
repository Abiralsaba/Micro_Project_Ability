"""
Project Ability — Chat Configuration
User/device mapping and MQTT settings.
Edit user names here freely.
"""

import os

# ═══════════════════════════════════════════════════════════
# USER ↔ DEVICE MAPPING
# ═══════════════════════════════════════════════════════════
# Each user owns one device. Change names as you like.

USERS = {
    "user_01": {
        "name": "Abir",
        "device": "glove_01",
        "device_type": "glove",
        "icon": "🧤",
    },
    "user_02": {
        "name": "Braille User",
        "device": "braille_01",
        "device_type": "braille",
        "icon": "⠿",
    },
    "user_03": {
        "name": "Voice User",
        "device": "voice_01",
        "device_type": "voice",
        "icon": "🔊",
    },
    "user_04": {
        "name": "Parkinson User",
        "device": "gaze_01",
        "device_type": "gaze",
        "icon": "👁",
    },
}

# Reverse lookup: device_id → user_id
DEVICE_TO_USER = {u["device"]: uid for uid, u in USERS.items()}

# ═══════════════════════════════════════════════════════════
# EMQX CLOUD MQTT (reuses existing braille_cloud credentials)
# ═══════════════════════════════════════════════════════════

MQTT_HOST = os.getenv("ABILITY_MQTT_HOST", "z91cfe11.ala.asia-southeast1.emqxsl.com")
MQTT_PORT = int(os.getenv("ABILITY_MQTT_PORT", "8883"))
MQTT_USERNAME = os.getenv("ABILITY_MQTT_USERNAME", "abir")
MQTT_PASSWORD = os.getenv("ABILITY_MQTT_PASSWORD", "1234")
MQTT_USE_TLS = os.getenv("ABILITY_MQTT_USE_TLS", "true").lower() not in ("0", "false", "no")

# ═══════════════════════════════════════════════════════════
# LOCAL MQTT FALLBACK (Mosquitto on Pi — uncomment to use)
# ═══════════════════════════════════════════════════════════
# MQTT_HOST = "192.168.4.1"
# MQTT_PORT = 1883
# MQTT_USERNAME = None
# MQTT_PASSWORD = None
# MQTT_USE_TLS = False

# ═══════════════════════════════════════════════════════════
# MQTT TOPICS (matches existing project architecture)
# ═══════════════════════════════════════════════════════════

# Cloud topics (v1 schema used by braille_cloud.ino + cloud_hub.py)
TOPIC_INPUT = "ability/v1/{device}/input"       # device publishes here
TOPIC_OUTPUT = "ability/v1/{device}/output"      # hub publishes here
TOPIC_STATUS = "ability/v1/{device}/status"      # device status

# Local topics (used by glove_mqtt.ino + braille_mqtt.ino)
TOPIC_LOCAL_GLOVE_TEXT = "ability/glove/text"
TOPIC_LOCAL_GLOVE_STATUS = "ability/glove/status"
TOPIC_LOCAL_BRAILLE_DISPLAY = "ability/braille/display"
TOPIC_LOCAL_BRAILLE_STATUS = "ability/braille/status"
TOPIC_LOCAL_VOICE_SPEAK = "ability/voice/speak"

# Chat-specific topic (new — does not conflict with existing)
TOPIC_CHAT_MESSAGE = "ability/v1/chat/{user_id}/message"

# ═══════════════════════════════════════════════════════════
# WEB SERVER
# ═══════════════════════════════════════════════════════════

WEB_HOST = "0.0.0.0"
WEB_PORT = 5050
