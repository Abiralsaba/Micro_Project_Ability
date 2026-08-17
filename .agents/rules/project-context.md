# Project Ability — Context Rule (Auto-Loaded)

## What This Project Is

**Project Ability** is a **research-level, modular cross-disability communication ecosystem**. It is a platform where each disabled person owns a **personal, specialized module** (only the hardware they need for their disability), and all modules connect to a **shared Central Hub** that handles format conversion, routing, and global connectivity. **Everyone can communicate with everyone, bidirectionally, from anywhere in the world.** The system automatically translates between formats (sign language ↔ Braille ↔ text ↔ speech ↔ avatars).

## Architecture — Modular Ecosystem (3-Tier)

**Tier 1 — Individual Modules** (personal, disability-specific, cheap):
- **Glove Module** ($70-90): ESP32 + flex sensors + IMU + RPi Zero 2W + camera + screen → for Deaf & Mute
- **Braille Module** ($20-30): ESP32 + 6 buttons + 2 servo motors + 2 cam-profiled racks (3D printed) + 6 gear-driven pins + speaker → for Blind
- **Gaze Module** ($50-65): RPi Zero 2W + NoIR camera + IR LEDs + screen + speaker → for Paralyzed & Parkinson's
- **Voice Module** ($15-25): ESP32 + mic + speaker → for Hearing/Sighted

**Tier 2 — Central Hub** ($150-200, shared per household/classroom):
- RPi 5 (8GB) + NVMe SSD + 7" touchscreen
- Pure processing + routing unit — no sensors or actuators
- Receives TEXT from modules → converts to receiver's format → delivers
- STT for Voice Module (audio at 32kbps, not video)
- Local MQTT broker + remote MQTT client + E2EE (AES-256-GCM)

**Tier 3 — Cloud Relay** (MQTT Broker on VPS):
- Routes encrypted TEXT between Central Hubs worldwide
- Never reads content (E2EE)

**Universal Intermediate:** TEXT. Every input encodes → text (on module). Hub converts text → receiver's format. **Zero video streaming. Only tiny processed payloads (~100 bytes per message).**

## 4 Module Types Serve 6 Disability Profiles

| Module | Users | Sends | Receives |
|---|---|---|---|
| Glove | Deaf, Mute | TEXT (hybrid fusion ML on module) | Avatar sign IDs / screen text |
| Braille | Blind | TEXT (button decode on module) | Braille byte encoding + TTS audio |
| Gaze | Paralyzed, Parkinson's | TEXT (MediaPipe Iris on module) | Screen text + TTS audio |
| Voice | Hearing | Audio stream to hub (hub does STT) | TTS audio |

## 5 Novel Research Contributions

1. **Cross-disability remote communication protocol** — no existing system does this (STRONGEST)
2. **6×6 communication matrix** — 30+ translation paths across 6 profiles (unprecedented)
3. **Modular ecosystem architecture** — specialized low-cost modules + shared hub (novel IoT topology)
4. **Tremor-tolerant gaze keyboard** — $50-65 module vs. Tobii $5000+; integrated into cross-disability IoT
5. **Hybrid sensor-vision fusion** — glove + MediaPipe on module = all 5 sign language parameters; no data leaves module until TEXT

## Target Publication
- **IEEE Internet of Things Journal** (IF 8.2, Q1)
- Paper title: "Ability: A Modular IoT Ecosystem for Cross-Disability Remote Communication..."
- Framing: Novel modular ecosystem architecture — NOT "smart glove"

## Key Files — READ THESE FIRST
- **`PROJECT_BRAIN.md`** — Quick reference + status + TODO tracker (15 sections)
- **`plan/MASTER_PLAN.md`** — Full research plan with all 30 paths, per-module BOM, architecture, roadmap
- `anti/novelty_analysis.md` — Literature review + publishability verdict
- `anti/project_ability_slide_deck.md` — 23-slide course presentation guide
- `anti/research_proposal_slides.md` — 11-slide Q1 journal pitch
- `anti/sign_language_fix.md` — Hybrid fusion technical deep-dive

## Rules for Working on This Project
- **Always** read `PROJECT_BRAIN.md` then `plan/MASTER_PLAN.md` at the start of any task.
- **Frame** everything as a modular cross-disability remote communication ecosystem.
- **Emphasize** bidirectional (everyone ↔ everyone), global (anywhere), and modular (buy only what you need).
- **Include** all 6 profiles — especially Parkinson's/neurodegenerative (key differentiator).
- **TEXT** is the universal intermediate — all translation goes through text.
- **Modules** do their own input processing locally → send only TEXT to the hub (privacy).
- **No video streaming** — only TEXT, audio (32kbps), and tiny command payloads travel over the network.
- This is **research-level** work — everything must be academically rigorous.
