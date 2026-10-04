#!/usr/bin/env python3
"""
Project Ability — Chat Interface Backend
Bridges the web chat UI with the existing MQTT ecosystem.

Runs on the laptop. Connects to EMQX cloud broker (same as braille_cloud.ino).
Provides a WhatsApp-style web interface at http://localhost:5050

Does NOT replace or modify existing hub/main.py or cloud_hub.py routing.
Sits alongside the existing system as an additional MQTT client.
"""

import json
import logging
import os
import ssl
import subprocess
import sys
import threading
import time
import uuid

from pathlib import Path

from flask import Flask, render_template, send_from_directory
from flask_socketio import SocketIO, emit
import paho.mqtt.client as mqtt

# ── Import config from same directory ────────────────────────
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from chat_config import (
    USERS, DEVICE_TO_USER,
    MQTT_HOST, MQTT_PORT, MQTT_USERNAME, MQTT_PASSWORD, MQTT_USE_TLS,
    TOPIC_STATUS, TOPIC_OUTPUT, TOPIC_INPUT,
    TOPIC_LOCAL_GLOVE_TEXT, TOPIC_LOCAL_GLOVE_STATUS,
    TOPIC_LOCAL_BRAILLE_DISPLAY, TOPIC_LOCAL_BRAILLE_STATUS,
    TOPIC_CHAT_MESSAGE,
    WEB_HOST, WEB_PORT,
)

# ═══════════════════════════════════════════════════════════
# LOGGING
# ═══════════════════════════════════════════════════════════

logging.basicConfig(
    level=logging.INFO,
    format="%(asctime)s [%(levelname)s] %(message)s",
    datefmt="%H:%M:%S",
)
log = logging.getLogger("ability-chat")

# ═══════════════════════════════════════════════════════════
# FLASK + SOCKETIO
# ═══════════════════════════════════════════════════════════

app = Flask(__name__, template_folder="templates")
app.config["SECRET_KEY"] = "ability-chat-secret"
app.config["TEMPLATES_AUTO_RELOAD"] = True
socketio = SocketIO(app, cors_allowed_origins="*", async_mode="threading")

PROJECT_ROOT = Path(__file__).resolve().parents[1]
SIGN_BUILD_DIR = PROJECT_ROOT / "Sign-Language-Toolkit-main" / "client" / "build"

# ═══════════════════════════════════════════════════════════
# APPLICATION STATE
# ═══════════════════════════════════════════════════════════

# Track which devices are online (device_id → last_seen timestamp)
device_online = {}
ONLINE_TIMEOUT = 30  # seconds — mark offline if no status for 30s

# Active conversation partner per user (user_id → target_user_id)
active_recipient = {}

# Message history per conversation pair (sorted key tuple → list of messages)
message_history = {}

# ═══════════════════════════════════════════════════════════
# TTS (Text-to-Speech) — uses macOS 'say' command
# ═══════════════════════════════════════════════════════════

def speak(text):
    """Speak text aloud using macOS built-in TTS."""
    try:
        subprocess.Popen(
            ["say", "-r", "180", text],
            stdout=subprocess.DEVNULL,
            stderr=subprocess.DEVNULL,
        )
        log.info("TTS: %s", text)
    except FileNotFoundError:
        log.warning("TTS not available (macOS 'say' command not found)")


# ═══════════════════════════════════════════════════════════
# HELPER FUNCTIONS
# ═══════════════════════════════════════════════════════════

def conversation_key(user_a, user_b):
    """Deterministic key for a conversation between two users."""
    return tuple(sorted([user_a, user_b]))


def get_user_by_name(name):
    """Find user_id by name (case-insensitive partial match)."""
    name_lower = name.lower().strip()
    for uid, u in USERS.items():
        if u["name"].lower() == name_lower:
            return uid
    # Partial match
    for uid, u in USERS.items():
        if name_lower in u["name"].lower():
            return uid
    return None


def is_device_online(device_id):
    """Check if a device has sent status recently."""
    last_seen = device_online.get(device_id, 0)
    return (time.time() - last_seen) < ONLINE_TIMEOUT


def get_all_user_status():
    """Build the full user list with online/offline status."""
    result = []
    for uid, u in USERS.items():
        result.append({
            "user_id": uid,
            "name": u["name"],
            "device": u["device"],
            "device_type": u["device_type"],
            "icon": u["icon"],
            "online": is_device_online(u["device"]),
            "active_recipient": active_recipient.get(uid),
        })
    return result


def store_message(sender_id, receiver_id, text):
    """Store a message in history and return the message dict."""
    key = conversation_key(sender_id, receiver_id)
    msg = {
        "id": str(uuid.uuid4())[:8],
        "sender_id": sender_id,
        "sender_name": USERS[sender_id]["name"],
        "receiver_id": receiver_id,
        "receiver_name": USERS[receiver_id]["name"],
        "text": text,
        "timestamp": time.time(),
        "time_str": time.strftime("%H:%M"),
    }
    message_history.setdefault(key, []).append(msg)
    return msg


def emit_message(msg):
    """Send one stored message to chat clients and, when applicable, sign UI."""
    socketio.emit("new_message", msg)
    receiver = USERS.get(msg["receiver_id"], {})
    if receiver.get("device_type") == "braille":
        # The next physical Braille-keyboard message automatically replies to
        # the person who most recently messaged this user.
        active_recipient[msg["receiver_id"]] = msg["sender_id"]
        socketio.emit("recipient_set", {
            "user_id": msg["receiver_id"],
            "target_id": msg["sender_id"],
            "target_name": USERS[msg["sender_id"]]["name"],
        })
    if receiver.get("device_type") == "glove":
        # The sign display receives the exact same message object/text as chat.
        socketio.emit("sign_message", msg)


# ═══════════════════════════════════════════════════════════
# MQTT CLIENT — connects to EMQX cloud (same as braille_cloud)
# ═══════════════════════════════════════════════════════════

mqtt_client = mqtt.Client(
    mqtt.CallbackAPIVersion.VERSION2,
    client_id=f"ability-chat-{uuid.uuid4().hex[:6]}",
    protocol=mqtt.MQTTv5,
)


def on_mqtt_connect(client, userdata, flags, reason_code, properties):
    if reason_code.is_failure:
        log.error("MQTT connection failed: %s", reason_code)
        return

    log.info("✓ Connected to MQTT broker %s:%d", MQTT_HOST, MQTT_PORT)

    # Subscribe to status topics for all devices
    for uid, u in USERS.items():
        device = u["device"]
        # Cloud status topic
        topic = TOPIC_STATUS.format(device=device)
        client.subscribe(topic, qos=1)
        log.info("  Subscribed: %s", topic)

        # Cloud output topic (to see messages arriving at devices)
        topic_out = TOPIC_OUTPUT.format(device=device)
        client.subscribe(topic_out, qos=1)
        log.info("  Subscribed: %s", topic_out)

    # Subscribe to cloud input wildcard (to intercept device → hub messages)
    client.subscribe("ability/v1/+/input", qos=1)
    log.info("  Subscribed: ability/v1/+/input")

    # Subscribe to local topics too (if broker bridges them)
    client.subscribe(TOPIC_LOCAL_GLOVE_TEXT, qos=0)
    client.subscribe(TOPIC_LOCAL_GLOVE_STATUS, qos=0)
    client.subscribe(TOPIC_LOCAL_BRAILLE_STATUS, qos=0)
    log.info("  Subscribed to local fallback topics")

    # Subscribe to chat message topics
    client.subscribe("ability/v1/chat/+/message", qos=1)
    log.info("  Subscribed: ability/v1/chat/+/message")

    # Notify web UI
    socketio.emit("mqtt_status", {"connected": True})


def on_mqtt_disconnect(client, userdata, disconnect_flags, reason_code, properties):
    log.warning("MQTT disconnected: %s", reason_code)
    socketio.emit("mqtt_status", {"connected": False})


def on_mqtt_message(client, userdata, message):
    """Handle all incoming MQTT messages."""
    topic = message.topic
    try:
        payload = message.payload.decode("utf-8", errors="replace")
    except Exception:
        return

    log.info("◄ MQTT [%s]: %s", topic, payload[:200])

    # ── Device status updates ────────────────────────────
    # Cloud status: ability/v1/{device}/status
    parts = topic.split("/")
    if len(parts) >= 4 and parts[0] == "ability" and parts[1] == "v1" and parts[3] == "status":
        device_id = parts[2]
        try:
            data = json.loads(payload)
        except json.JSONDecodeError:
            data = {}

        is_online = data.get("online", True)
        if is_online:
            device_online[device_id] = time.time()
        else:
            device_online.pop(device_id, None)

        # Notify web UI of status change
        socketio.emit("user_status", get_all_user_status())
        log.info("  Device %s → %s", device_id, "ONLINE" if is_online else "OFFLINE")
        return

    # Local status: ability/glove/status or ability/braille/status
    if topic == TOPIC_LOCAL_GLOVE_STATUS:
        device_online["glove_01"] = time.time()
        socketio.emit("user_status", get_all_user_status())
        return
    if topic == TOPIC_LOCAL_BRAILLE_STATUS:
        device_online["braille_01"] = time.time()
        socketio.emit("user_status", get_all_user_status())
        return

    # ── Device input messages (device → hub) ─────────────
    # Cloud: ability/v1/{device}/input
    if len(parts) >= 4 and parts[3] == "input":
        device_id = parts[2]
        user_id = DEVICE_TO_USER.get(device_id)
        if not user_id:
            return

        try:
            data = json.loads(payload)
        except json.JSONDecodeError:
            data = {"text": payload}

        text = data.get("text", "")
        if not text:
            return

        # If this user has an active recipient, route as chat message
        recipient = active_recipient.get(user_id)
        if recipient:
            msg = store_message(user_id, recipient, text)
            emit_message(msg)
            log.info("  Chat: %s → %s: %s",
                      USERS[user_id]["name"], USERS[recipient]["name"], text)

            # Also forward to the recipient's device via existing MQTT
            deliver_to_device(recipient, text, user_id)
        else:
            # No active recipient — just show in the UI as unrouted
            socketio.emit("device_text", {
                "user_id": user_id,
                "name": USERS[user_id]["name"],
                "text": text,
                "timestamp": time.time(),
            })
        return

    # Local glove text: ability/glove/text
    if topic == TOPIC_LOCAL_GLOVE_TEXT:
        try:
            data = json.loads(payload)
        except json.JSONDecodeError:
            data = {"text": payload}

        text = data.get("text", "")
        if not text:
            return

        user_id = DEVICE_TO_USER.get("glove_01")
        if user_id:
            device_online["glove_01"] = time.time()
            recipient = active_recipient.get(user_id)
            if recipient:
                msg = store_message(user_id, recipient, text)
                emit_message(msg)
                deliver_to_device(recipient, text, user_id)
            else:
                socketio.emit("device_text", {
                    "user_id": user_id,
                    "name": USERS[user_id]["name"],
                    "text": text,
                    "timestamp": time.time(),
                })
        return

    # ── Chat messages (from web UI to another user) ──────
    if len(parts) >= 5 and parts[2] == "chat" and parts[4] == "message":
        # Already handled by the web UI directly
        return


def deliver_to_device(target_user_id, text, source_user_id):
    """
    Send a message to the target user's physical device.
    Uses the existing MQTT topics that the devices already listen to.
    """
    target = USERS.get(target_user_id)
    if not target:
        return

    device = target["device"]
    device_type = target["device_type"]
    source_name = USERS.get(source_user_id, {}).get("name", "Unknown")

    if device_type == "braille":
        # Braille module listens on ability/v1/braille_01/output
        # It accepts {"text":"..."} or {"braille":[...]}
        # Send as text — the module handles display
        payload = json.dumps({
            "version": 1,
            "message_id": str(uuid.uuid4()),
            "source": USERS[source_user_id]["device"],
            "target": device,
            "type": "text",
            "text": text,
            "timestamp": int(time.time()),
        })
        topic = TOPIC_OUTPUT.format(device=device)
        mqtt_client.publish(topic, payload, qos=1)
        log.info("  → Delivered to Braille: %s", text)

        # Also announce via TTS on laptop
        speak(f"New message from {source_name}")

    elif device_type == "glove":
        # Glove listens on ability/glove/output (local) — future
        # For now, the message appears in the web UI
        log.info("  → Glove user sees message in web UI: %s", text)

    elif device_type == "voice":
        # Voice module — future TTS delivery
        log.info("  → Voice user sees message in web UI: %s", text)


# ═══════════════════════════════════════════════════════════
# VOICE COMMAND PROCESSING (for Braille user accessibility)
# ═══════════════════════════════════════════════════════════

def process_voice_command(command_text, from_user_id=None):
    """
    Process a voice command, primarily for the Braille user.
    Returns a response string and optional action.
    """
    cmd = command_text.lower().strip()
    response = None
    action = None

    # "message <name>" — set active recipient
    if cmd.startswith("message "):
        name = cmd[8:].strip()
        target_uid = get_user_by_name(name)
        if target_uid:
            if from_user_id:
                active_recipient[from_user_id] = target_uid
            target_name = USERS[target_uid]["name"]
            response = f"{target_name} selected."
            action = {"type": "set_recipient", "user_id": from_user_id, "target": target_uid}
            speak(response)
        else:
            response = f"Contact {name} not found."
            speak(response)

    # "reply" — keep current recipient
    elif cmd == "reply":
        if from_user_id and active_recipient.get(from_user_id):
            target_name = USERS[active_recipient[from_user_id]]["name"]
            response = f"Replying to {target_name}."
            speak(response)
        else:
            response = "No active conversation to reply to."
            speak(response)

    # "read contacts" — list all users
    elif cmd in ("read contacts", "contacts", "list contacts"):
        names = [u["name"] for uid, u in USERS.items() if uid != from_user_id]
        response = "Contacts: " + ", ".join(names)
        speak(response)

    # "who am i talking to?"
    elif cmd in ("who am i talking to", "who am i talking to?", "current"):
        if from_user_id and active_recipient.get(from_user_id):
            target_name = USERS[active_recipient[from_user_id]]["name"]
            response = f"You are talking to {target_name}."
        else:
            response = "No active conversation."
        speak(response)

    # "status" — who is online
    elif cmd in ("status", "who is online", "online"):
        online_names = []
        for uid, u in USERS.items():
            if is_device_online(u["device"]):
                online_names.append(u["name"])
        if online_names:
            response = "Online: " + ", ".join(online_names)
        else:
            response = "No devices currently online."
        speak(response)

    else:
        response = f"Unknown command: {command_text}"

    return response, action


# ═══════════════════════════════════════════════════════════
# FLASK ROUTES
# ═══════════════════════════════════════════════════════════

@app.route("/")
def index():
    return render_template("chat.html", users=USERS)


@app.route("/sign/")
def sign_display():
    """Serve the compiled sign-language toolkit on a separate page."""
    return send_from_directory(SIGN_BUILD_DIR, "index.html")


@app.route("/sign/<path:asset_path>")
def sign_assets(asset_path):
    return send_from_directory(SIGN_BUILD_DIR, asset_path)


# ═══════════════════════════════════════════════════════════
# SOCKETIO EVENTS (Web UI ↔ Backend)
# ═══════════════════════════════════════════════════════════

@socketio.on("connect")
def handle_connect():
    """Client connected — send current state."""
    emit("user_status", get_all_user_status())
    emit("mqtt_status", {"connected": mqtt_client.is_connected()})
    log.info("Web client connected")


@socketio.on("get_status")
def handle_get_status():
    """Client requests current user/device status."""
    emit("user_status", get_all_user_status())


@socketio.on("select_user")
def handle_select_user(data):
    """Web UI user selects which user they are chatting 'as'."""
    # The web UI allows viewing any user's perspective
    pass


@socketio.on("set_recipient")
def handle_set_recipient(data):
    """Set active conversation: user_id wants to talk to target_id."""
    if not isinstance(data, dict):
        return {"ok": False, "error": "invalid request"}
    user_id = data.get("user_id")
    target_id = data.get("target_id")
    if user_id in USERS and target_id in USERS and user_id != target_id:
        active_recipient[user_id] = target_id
        log.info("Chat: %s → %s", USERS[user_id]["name"], USERS[target_id]["name"])
        key = conversation_key(user_id, target_id)
        return {
            "ok": True,
            "user_id": user_id,
            "target_id": target_id,
            "target_name": USERS[target_id]["name"],
            "messages": message_history.get(key, []),
        }
    return {"ok": False, "error": "invalid sender or recipient"}


@socketio.on("send_message")
def handle_send_message(data):
    """User sends a chat message from the web UI."""
    if not isinstance(data, dict):
        return
    sender_id = data.get("sender_id")
    receiver_id = data.get("receiver_id")
    raw_text = data.get("text", "")
    text = raw_text.strip() if isinstance(raw_text, str) else ""

    if (not text or sender_id not in USERS or receiver_id not in USERS
            or sender_id == receiver_id):
        return

    # Store message
    msg = store_message(sender_id, receiver_id, text)

    # Broadcast to all web clients
    emit_message(msg)

    # Set active recipient
    active_recipient[sender_id] = receiver_id

    # Deliver to the receiver's physical device via MQTT
    deliver_to_device(receiver_id, text, sender_id)

    # If receiver has the sender as active, update their recipient in backend
    receiver_device_type = USERS[receiver_id].get("device_type")
    if receiver_device_type == "braille":
        active_recipient[receiver_id] = sender_id

    log.info("Message: %s → %s: %s",
             USERS[sender_id]["name"], USERS[receiver_id]["name"], text)


@socketio.on("get_sign_state")
def handle_get_sign_state():
    """Give a newly opened sign display the latest message for a glove user."""
    received = [
        msg
        for messages in message_history.values()
        for msg in messages
        if USERS.get(msg["receiver_id"], {}).get("device_type") == "glove"
    ]
    emit("sign_state", {"message": max(received, key=lambda msg: msg["timestamp"]) if received else None})


@socketio.on("voice_command")
def handle_voice_command(data):
    """Process a voice command from the web UI."""
    command = data.get("command", "")
    from_user = data.get("from_user")

    response, action = process_voice_command(command, from_user)

    emit("voice_response", {
        "response": response,
        "action": action,
    })

    if action and action.get("type") == "set_recipient":
        emit("recipient_set", {
            "user_id": action["user_id"],
            "target_id": action["target"],
            "target_name": USERS[action["target"]]["name"],
        })


@socketio.on("simulate_device_event")
def handle_simulate_device_event(data):
    """Simulate a hardware event (glove gesture, braille status, voice input)."""
    device_id = data.get("device_id")
    event_type = data.get("type", "message")
    user_id = DEVICE_TO_USER.get(device_id)

    if not user_id and device_id not in USERS:
        if device_id in USERS:
            user_id = device_id
            device_id = USERS[user_id]["device"]
        else:
            return

    if event_type == "status":
        online = data.get("online", True)
        if online:
            device_online[device_id] = time.time()
        else:
            device_online.pop(device_id, None)
        socketio.emit("user_status", get_all_user_status())
        log.info("[Simulation] Device %s online=%s", device_id, online)

    elif event_type == "message":
        text = data.get("text", "").strip()
        if not text:
            return

        # Determine target_id: MUST NEVER BE user_id itself!
        target_id = data.get("target_id")
        if not target_id or target_id == user_id:
            target_id = active_recipient.get(user_id)
            if not target_id or target_id == user_id:
                for other_uid in USERS:
                    if other_uid != user_id:
                        target_id = other_uid
                        break

        if target_id and target_id != user_id:
            msg = store_message(user_id, target_id, text)
            emit_message(msg)
            deliver_to_device(target_id, text, user_id)
            log.info("[Simulation] %s (%s) sent message to %s: %s",
                     user_id, device_id, target_id, text)



# ═══════════════════════════════════════════════════════════
# MQTT SETUP & START
# ═══════════════════════════════════════════════════════════

def setup_mqtt():
    """Configure and connect the MQTT client."""
    if MQTT_USERNAME:
        mqtt_client.username_pw_set(MQTT_USERNAME, MQTT_PASSWORD)

    if MQTT_USE_TLS:
        mqtt_client.tls_set(cert_reqs=ssl.CERT_REQUIRED, tls_version=ssl.PROTOCOL_TLS)

    mqtt_client.on_connect = on_mqtt_connect
    mqtt_client.on_disconnect = on_mqtt_disconnect
    mqtt_client.on_message = on_mqtt_message
    mqtt_client.reconnect_delay_set(min_delay=1, max_delay=30)

    log.info("Connecting to MQTT broker %s:%d ...", MQTT_HOST, MQTT_PORT)
    try:
        mqtt_client.connect(MQTT_HOST, MQTT_PORT, keepalive=60)
        mqtt_client.loop_start()
    except Exception as e:
        log.error("MQTT connection failed: %s", e)
        log.info("Chat will work without MQTT — devices won't be detected")


def background_status_monitor():
    """Periodically check device online timeouts and broadcast status changes."""
    prev_status = None
    while True:
        time.sleep(2)
        try:
            current = {u["device"]: is_device_online(u["device"]) for u in USERS.values()}
            if current != prev_status:
                prev_status = current
                socketio.emit("user_status", get_all_user_status())
                log.info("Device status updated: %s", current)
        except Exception:
            pass


# ═══════════════════════════════════════════════════════════
# MAIN
# ═══════════════════════════════════════════════════════════

def main():
    log.info("═" * 50)
    log.info("  Project Ability — Chat Interface")
    log.info("  http://localhost:%d", WEB_PORT)
    log.info("═" * 50)

    setup_mqtt()

    # Start periodic offline checker
    monitor_thread = threading.Thread(target=background_status_monitor, daemon=True)
    monitor_thread.start()

    socketio.run(
        app,
        host=WEB_HOST,
        port=WEB_PORT,
        debug=False,
        allow_unsafe_werkzeug=True,
    )


if __name__ == "__main__":
    main()
