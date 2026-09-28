# Central Hub — The Brain of the Ecosystem

> **Hardware:** Raspberry Pi 5 (8GB) + NVMe SSD + 7" Touchscreen
> **Cost:** ~$150–200 (shared per household/classroom/clinic)

## What the Central Hub Does

The Central Hub is a **shared processing and routing unit** — like a router for disability communication. It contains **no sensors and no actuators** (those are on individual modules). Its job:

1. **Receives TEXT** from all connected modules (via local WiFi MQTT)
2. **STT** for Voice Module (receives audio 32kbps → Vosk/Whisper → TEXT)
3. **Converts TEXT** → receiver's format (Braille bytes / Avatar IDs / TTS audio / display text)
4. **Routes locally** (Module A → Hub → Module B on same network)
5. **Routes remotely** (Encrypt → MQTT cloud → Remote Hub)
6. **Manages** user profiles, contacts, conversation history, health alerts

## Subdirectories

| Directory | Purpose |
|---|---|
| `core/` | Hub engine — main loop, message router, module registry |
| `converters/` | Format converters: `text_to_braille.py`, `text_to_avatar.py`, `text_to_tts.py` |
| `stt/` | Speech-to-Text engine wrapper (Vosk / Whisper offline) |
| `tts/` | Text-to-Speech engine wrapper (Piper offline) |
| `networking/` | MQTT client (local + remote), AMP protocol handler, E2EE (AES-256-GCM) |
| `ui/` | Admin touchscreen interface (user profiles, contacts, status dashboard) |
| `health/` | Health data monitor (SpO2, heart rate, fall detection alerts) |
| `config/` | YAML configuration files |

## Architecture

```
                    ┌───────────────────────────────────┐
                    │         CENTRAL HUB (RPi 5)       │
                    │                                   │
  Modules ──WiFi──► │  Local MQTT ──► Router ──► Format │ ──WiFi──► Modules
                    │  Broker         Engine    Convert  │
                    │                   │               │
                    │                   ▼               │
                    │            Remote MQTT ──► Cloud   │
                    │            + AES-256-GCM          │
                    └───────────────────────────────────┘
```
