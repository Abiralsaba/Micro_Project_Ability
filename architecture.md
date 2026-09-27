# Project Ability — System Architecture

> **Last Updated:** 2026-09-27

---

## Overview

```
┌─────────────────────────────────────────────────────────────────┐
│                    WiFi Network: "Ability"                       │
│                    Password: "ability123"                        │
│                                                                 │
│   ┌──────────┐   ┌──────────┐   ┌──────────┐   ┌──────────┐   │
│   │ 🧤 Glove │   │ ⠿ Braille│   │ 🔊 Voice │   │ 👁 Gaze  │   │
│   │ ESP32-S3 │   │ ESP32    │   │ ESP32-S3 │   │ RPi Zero │   │
│   │          │   │          │   │          │   │  2W      │   │
│   └────┬─────┘   └────┬─────┘   └────┬─────┘   └────┬─────┘   │
│        │ MQTT         │ MQTT         │ MQTT         │ MQTT     │
│        │              │              │              │          │
│   ┌────▼──────────────▼──────────────▼──────────────▼────┐     │
│   │                                                       │     │
│   │              🧠 Raspberry Pi 5                        │     │
│   │              192.168.4.1                              │     │
│   │                                                       │     │
│   │   ┌────────────┐  ┌────────────┐  ┌────────────┐    │     │
│   │   │ WiFi AP    │  │ Mosquitto  │  │ Hub Engine │    │     │
│   │   │ (hostapd)  │  │ (MQTT)     │  │ (main.py)  │    │     │
│   │   └────────────┘  └────────────┘  └────────────┘    │     │
│   │   ┌────────────┐  ┌────────────┐  ┌────────────┐    │     │
│   │   │ DHCP       │  │ Converters │  │ STT / TTS  │    │     │
│   │   │ (dnsmasq)  │  │ (Python)   │  │ (future)   │    │     │
│   │   └────────────┘  └────────────┘  └────────────┘    │     │
│   │                                                       │     │
│   └───────────────────────────────────────────────────────┘     │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

---

## Tier 1: Modules (ESP32s)

Each module is a personal device owned by one user. It connects to the Pi's WiFi and communicates via MQTT.

| Module | MCU | Users | Input | Output |
|--------|-----|-------|-------|--------|
| **Glove** | ESP32-S3 Super Mini | Deaf, Mute | 5 flex sensors + BNO055 IMU → ASL classifier → TEXT | Screen (avatar/text) |
| **Braille** | ESP32-WROOM-32 | Blind | 6 braille buttons (future) | 8 MG90S servos → 4 braille cells (cam mechanism) |
| **Voice** | ESP32-S3 | Hearing | INMP441 mic → audio stream | Speaker (TTS playback) |
| **Gaze** | RPi Zero 2W | Paralyzed, Parkinson's | NoIR camera → eye tracking → TEXT | Screen (keyboard UI) |

---

## Tier 2: Central Hub (Raspberry Pi 5)

The Pi does ALL processing. Modules are dumb endpoints.

| Service | Software | Purpose |
|---------|----------|---------|
| WiFi AP | hostapd | Creates "Ability" network, no router needed |
| DHCP | dnsmasq | Assigns IPs to connected modules (192.168.4.10-50) |
| MQTT Broker | Mosquitto | Receives/routes all messages on port 1883 |
| Hub Engine | `main.py` (Python) | Subscribes to all topics, routes between modules |
| Converters | Python | Text → braille commands, text → TTS audio |
| STT | Vosk / Whisper (future) | Voice audio → text |
| TTS | Piper (future) | Text → speech audio |

---

## Tier 3: Cloud Relay (Future)

| Component | Purpose |
|-----------|---------|
| Mosquitto VPS | Routes encrypted messages between remote hubs |
| TLS 1.3 | Transport encryption |
| AES-256-GCM | End-to-end message encryption |

```
Hub A (Dhaka) ──► MQTT Cloud ──► Hub B (Sylhet)
                  (encrypted)
```

---

## MQTT Topic Design

### Topic Structure: `ability/{module}/{action}`

| Topic | Direction | Publisher | Subscriber | Payload Example |
|-------|-----------|-----------|------------|-----------------|
| `ability/glove/text` | Module → Hub | Glove ESP32 | Hub | `{"text":"HELP","user":"glove_01"}` |
| `ability/glove/status` | Module → Hub | Glove ESP32 | Hub | `{"online":true,"imu_ok":true}` |
| `ability/braille/display` | Hub → Module | Hub | Braille ESP32 | `{"text":"HELP","from":"glove_01"}` |
| `ability/braille/status` | Module → Hub | Braille ESP32 | Hub | `{"modules_ok":[true,true,true,true]}` |
| `ability/voice/speak` | Hub → Module | Hub | Voice ESP32 | `{"text":"HELP","from":"glove_01"}` |
| `ability/voice/audio` | Module → Hub | Voice ESP32 | Hub | `{"audio_b64":"..."}` |
| `ability/hub/command` | Any → Hub | Any | Hub | `{"cmd":"home","target":"braille"}` |
| `ability/hub/status` | Hub → All | Hub | All | `{"online":true,"modules":3}` |

---

## Data Flow: All Communication Paths

### Path 1: Glove → Braille (Primary Demo)

```
Deaf user signs "HELP"
         │
         ▼
┌─────────────────┐
│ Glove ESP32-S3  │  ASL classifier detects H, E, L, P
│ Sentence: HELP  │  User does SEND gesture (palm down)
│                 │  Publishes to MQTT
└────────┬────────┘
         │  ability/glove/text → {"text":"HELP"}
         ▼
┌─────────────────┐
│ Pi Hub          │  Receives text
│ main.py         │  Routes to braille topic
└────────┬────────┘
         │  ability/braille/display → {"text":"HELP"}
         ▼
┌─────────────────┐
│ Braille ESP32   │  Parses JSON, extracts "HELP"
│ 4 Modules       │  M1=H, M2=E, M3=L, M4=P
│                 │  Hold 5 seconds → home
└─────────────────┘
```

### Path 2: Glove → Voice

```
Deaf user signs "WATER"
         │
         ▼
Glove ESP32 → MQTT → Pi Hub → Text-to-Speech (Piper)
                                      │
                                      ▼
                          Pi publishes audio to MQTT
                                      │
                                      ▼
                          Voice ESP32 plays through speaker
```

### Path 3: Voice → Braille

```
Hearing user speaks "hello"
         │
         ▼
Voice ESP32 → audio stream → Pi Hub → Speech-to-Text (Vosk)
                                              │
                                              ▼
                                    Pi gets text "hello"
                                              │
                                              ▼
                                    Pi publishes to braille topic
                                              │
                                              ▼
                                    Braille ESP32 displays "hello"
```

### Path 4: Gaze → Braille

```
Paralyzed user types "help me" via eye-gaze keyboard
         │
         ▼
RPi Zero 2W → MQTT → Pi Hub → ability/braille/display
                                      │
                                      ▼
                          Braille ESP32 displays "help me"
```

---

## Network Configuration

### IP Assignments

| Device | IP | Role |
|--------|-----|------|
| Raspberry Pi 5 | 192.168.4.1 | WiFi AP + MQTT Broker + Hub Engine |
| Glove ESP32-S3 | DHCP (192.168.4.10-50) | Sign language input |
| Braille ESP32 | DHCP (192.168.4.10-50) | Braille tactile output |
| Voice ESP32-S3 | DHCP (192.168.4.10-50) | Audio input/output |
| RPi Zero 2W | DHCP (192.168.4.10-50) | Glove camera + ML |

### Credentials

```
WiFi SSID:       Ability
WiFi Password:   ability123
MQTT Host:       192.168.4.1
MQTT Port:       1883
MQTT Auth:       Anonymous (local network)
```

---

## Hardware Pin Maps

### Braille Module (ESP32-WROOM-32)

| GPIO | Servo | Module |
|------|-------|--------|
| 13 | Left (Dots 1,2,3) | Module 1 |
| 12 | Right (Dots 4,5,6) | Module 1 |
| 14 | Left | Module 2 |
| 27 | Right | Module 2 |
| 26 | Left | Module 3 |
| 25 | Right | Module 3 |
| 33 | Left | Module 4 |
| 32 | Right | Module 4 |

Servo pulse: 500-2400μs, 50Hz PWM

### Glove Module (ESP32-S3 Super Mini)

| GPIO | Component |
|------|-----------|
| 1 | Flex Sensor: Thumb |
| 2 | Flex Sensor: Index |
| 3 | Flex Sensor: Middle |
| 4 | Flex Sensor: Ring |
| 5 | Flex Sensor: Pinky |
| 8 | BNO055 SDA |
| 9 | BNO055 SCL |

---

## Software Stack

| Layer | Technology | Location |
|-------|-----------|----------|
| ESP32 Firmware | C/C++ (Arduino) | `modules/*/firmware/` |
| Hub Engine | Python 3 | `hub/core/main.py` |
| MQTT Broker | Mosquitto | Pi system service |
| WiFi AP | hostapd + dnsmasq | Pi system service |
| STT (future) | Vosk / Whisper | `hub/stt/` |
| TTS (future) | Piper | `hub/tts/` |

### Arduino Libraries

| Library | Version | Used By |
|---------|---------|---------|
| ESP32Servo | Latest | Braille |
| PubSubClient | 2.8+ | Braille, Glove |
| ArduinoJson | 7.x | Braille, Glove |

### Python Dependencies

| Package | Used For |
|---------|----------|
| paho-mqtt | MQTT client on Hub |
| pyyaml | Config parsing |

---

## File Structure

```
Micro_Project_Ability/
├── PROJECT_BRAIN.md              ← Master reference & TODO
├── SETUP_GUIDE.md                ← Step-by-step setup instructions
├── architecture.md               ← THIS FILE
│
└── CodeBase/
    ├── hub/
    │   ├── core/
    │   │   └── main.py           ← Hub engine (MQTT router)
    │   ├── converters/           ← Text → Braille / TTS / Avatar
    │   ├── stt/                  ← Speech-to-Text (future)
    │   ├── tts/                  ← Text-to-Speech (future)
    │   ├── networking/           ← MQTT + E2EE handlers
    │   ├── config/
    │   │   └── config.example.yaml
    │   └── ui/                   ← Admin touchscreen UI (future)
    │
    ├── modules/
    │   ├── braille/firmware/braille_module/
    │   │   ├── braille_mqtt.ino  ← MQTT version (production)
    │   │   ├── demomodule.ino    ← Standalone WiFi AP version
    │   │   ├── brailv1.ino       ← Single module reference
    │   │   └── v2working.ino     ← Single module alternate
    │   │
    │   ├── glove/firmware/glove_flex_imu/
    │   │   ├── glove_mqtt.ino    ← MQTT version (production)
    │   │   └── glove_flex_imu.ino ← Standalone Serial version
    │   │
    │   ├── voice/firmware/       ← (future)
    │   └── gaze/src/             ← (future)
    │
    ├── scripts/setup/
    │   └── setup_hub.sh          ← Pi automated setup
    │
    ├── docs/
    │   └── SETUP_GUIDE.md        ← Detailed setup guide
    │
    └── cloud/mqtt/
        └── mosquitto.conf        ← Cloud broker config (future)
```

---

## Design Decisions

| Decision | Choice | Reason |
|----------|--------|--------|
| Protocol | MQTT over ROS2 | Native ESP32 support, 5MB RAM vs 500MB, 1 hour setup vs weeks |
| Network | Pi as WiFi AP | No external router needed, works anywhere |
| Hub language | Python | Fast development, paho-mqtt, STT/TTS libraries |
| Servo control | Individual objects + switch-case | ESP32 LEDC channel conflicts with Servo arrays |
| Intermediate format | TEXT | Reduces N×N paths to N encoders + N decoders |
| ML location | On modules (not hub) | Privacy — no raw sensor data leaves the device |
| Braille mechanism | Cam-based rack & pinion | 2 servos per cell, 8 positions each, 3D printable |
