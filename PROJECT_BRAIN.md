# 🧠 PROJECT ABILITY — BRAIN (Master Reference & TODO)

> **Last Updated:** 2026-08-17
> **Status:** 📝 Planning & Proposal Phase — No prototype built yet
> **Owner:** Abiral Saba
> **Full Plan:** See `plan/MASTER_PLAN.md` for comprehensive details with all 30 translation paths

---

## 1. WHAT IS PROJECT ABILITY?

**Project Ability** is a **research-level, modular cross-disability communication ecosystem**. Each disabled person owns a **personal, specialized module** (only the hardware they need for their disability), and all modules connect to a **shared Central Hub** that handles format conversion, routing, and global connectivity.

### The Core Principle — EVERYONE ↔ EVERYONE
**Any person with any disability can send a message to any other person with any other disability, anywhere in the world, and the system translates it into the receiver's native format automatically.**

- Deaf signs → Blind feels Braille. Blind types Braille → Deaf sees 3D Avatar signing.
- Parkinson's patient gazes at keyboard → Deaf grandchild sees avatar. Deaf grandchild signs back → Parkinson's patient hears speech.
- Paralyzed person blinks to type → Mute person reads screen. Mute person gestures → Paralyzed person hears TTS.
- Hearing person speaks → Blind person feels Braille. Blind person types Braille → Hearing person hears TTS.
- **30+ unique cross-disability paths, all bidirectional, all working locally AND remotely.**

### The Ecosystem Philosophy
> *"Buy your module. Join the network. Talk to anyone."*
- Each person buys **only the module for their disability** — nothing more
- No wasted hardware (blind person has no camera or screen they'll never use)
- All modules connect to a **shared Central Hub** (like a router for disability communication)
- Central Hubs connect globally via encrypted cloud relay

### What It Is NOT / What It IS
- ❌ NOT "just another smart glove" / ✅ A **cross-disability remote communication ecosystem**
- ❌ NOT a single-disability device / ✅ **6 disability profiles, 30+ translation paths**
- ❌ NOT a monolithic box everyone carries / ✅ **Modular: buy only what you need ($15–90)**
- ❌ NOT local-only / ✅ **Global via encrypted MQTT cloud relay**
- ❌ NOT a concept paper / ✅ Must have **working prototype** for Q1 publication

---

## 2. THE 6 DISABILITY PROFILES & THEIR I/O

| # | Profile | Input (How They SEND) | Output (How They RECEIVE) |
|---|---|---|---|
| 1 | **Deaf / Hard of Hearing** | Sign Language via Sensor Glove + Camera (Hybrid Fusion) | 3D Sign Language Avatar on screen |
| 2 | **Blind / Low Vision** | Braille buttons (6 keys, Perkins layout) | Braille rack-and-pinion gear pins (physical dots) + TTS audio |
| 3 | **Mute / Speech Impaired** | Gestures via Sensor Glove + Camera | Text on screen (they can read) |
| 4 | **Paralyzed (ALS, Spinal)** | Eye-Gaze cursor + Blink-to-Click | Text on screen + TTS audio |
| 5 | **Parkinson's / Neurodegenerative** | Gaze Keyboard (tremor-tolerant, dwell-click) | Text on screen + TTS audio |
| 6 | **Hearing / Sighted** | Voice via microphone (STT on hub) | Text / Speech (any format) |

---

## 3. THE UNIVERSAL TRANSLATION ARCHITECTURE

**TEXT is the universal intermediate.** Every input modality encodes TO text (on the module). Every output modality renders FROM text (Central Hub converts and delivers to target module).

```
  [Sign Language] ──►┐                          ┌──► [3D Sign Avatar]
  [Braille Input] ──►┤                          ├──► [Braille Gear Pins]
  [Gestures]      ──►┤  TEXT (universal)        ├──► [TTS Speech]
  [Eye-Gaze]      ──►┤  intermediate            ├──► [Screen Text]
  [Gaze Keyboard] ──►┤  (via Central Hub)       │
  [Voice / STT]   ──►┘                          │

  6 Input Encoders        Central Hub         4 Output Renderers
  (on individual modules) (converts & routes) (delivered to target modules)
```

**6 encoders + 4 renderers = 10 modules** (not 30 separate translators)

---

## 4. MODULAR ECOSYSTEM ARCHITECTURE (3-Tier)

```
  TIER 1: INDIVIDUAL MODULES (Personal, Disability-Specific)
  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐
  │ Glove Module │  │ Braille Mod  │  │ Gaze Module  │  │ Voice Module │
  │ (Deaf/Mute)  │  │ (Blind)      │  │ (Paralyzed/  │  │ (Hearing)    │
  │ ~$70-90      │  │ ~$30-40      │  │  Parkinson's)│  │ ~$15-25      │
  │              │  │              │  │ ~$50-65      │  │              │
  │ ESP32+Glove  │  │ ESP32        │  │ RPi Zero 2W  │  │ ESP32+Mic   │
  │ +RPiZero+Cam │  │ +Buttons     │  │ +NoIR Camera │  │ +Speaker     │
  │ +Small Screen│  │ +Gear Pins   │  │ +Screen      │  │              │
  │              │  │ +Speaker     │  │ +Speaker     │  │              │
  └──────┬───────┘  └──────┬───────┘  └──────┬───────┘  └──────┬───────┘
         │ WiFi            │ WiFi            │ WiFi            │ WiFi
         ▼                 ▼                 ▼                 ▼
  ┌─────────────────────────────────────────────────────────────────────┐
  │  TIER 2: CENTRAL HUB (Shared per household/classroom/clinic)       │
  │  RPi 5 (8GB) + NVMe SSD + 7" Touchscreen | ~$150-200             │
  │  Format conversion • Routing • STT/TTS • E2EE • User management   │
  └──────────────────────────────┬──────────────────────────────────────┘
                                 │ Internet
                                 ▼
  ┌─────────────────────────────────────────────────────────────────────┐
  │  TIER 3: CLOUD RELAY — MQTT Broker (Mosquitto VPS)                 │
  │  Routes encrypted TEXT between Central Hubs worldwide               │
  │  TLS 1.3 + AES-256-GCM E2EE • QoS 2 • Offline queuing            │
  └─────────────────────────────────────────────────────────────────────┘
```

### The 4 Module Types

| Module | Users | Key Hardware | Local Processing | Sends to Hub | Receives from Hub |
|---|---|---|---|---|---|
| **Glove** | Deaf, Mute | ESP32 + sensors + RPi Zero 2W + camera + screen | Hybrid fusion ML → TEXT | TEXT (~100 bytes) | Avatar sign IDs / Display text |
| **Braille** | Blind | ESP32 + 6 buttons + rack-and-pinion gear pins + speaker | Braille decode → TEXT | TEXT (~100 bytes) | Braille byte encoding + TTS audio |
| **Gaze** | Paralyzed, Parkinson's | RPi Zero 2W + NoIR camera + IR LEDs + screen + speaker | MediaPipe Iris → TEXT | TEXT (~100 bytes) | Display text + TTS audio |
| **Voice** | Hearing/Sighted | ESP32 + mic + speaker | — | Audio (32 kbps) | TTS audio |

### What the Central Hub Does
| Responsibility | Details |
|---|---|
| **Receives TEXT** | From all modules via local WiFi/MQTT |
| **STT** | Voice Module audio → Vosk/Whisper → TEXT |
| **Format Conversion** | TEXT → Braille encoding / Avatar sign IDs / TTS audio / display text |
| **Local Routing** | Module A → Hub → Module B (same network) |
| **Remote Routing** | Encrypt → MQTT → Cloud → Remote Hub → Remote Module |
| **Management** | User profiles, contacts, conversation history, health alerts |
| **E2EE** | AES-256-GCM encrypt/decrypt for remote messages |

### What Travels Over the Network (ALL LIGHTWEIGHT)
| Data | Size | Notes |
|---|---|---|
| TEXT message | ~100 bytes | Module → Hub or Hub → Hub |
| Audio stream | 32 kbps | Voice Module → Hub only (lighter than a phone call) |
| Braille encoding | ~20 bytes | Hub → Blind Module |
| Avatar commands | ~100 bytes | Hub → Deaf Module (sign IDs, not video) |
| TTS audio | ~50 kbps | Hub → modules with speakers |
| **Zero video. Zero raw sensor data.** | | Only tiny processed payloads |

---

## 5. THE 5 NOVEL RESEARCH CONTRIBUTIONS

1. **🏆 Cross-Disability Remote Communication Protocol** — No existing system does modality-agnostic remote messaging (STRONGEST NOVELTY)
2. **🏆 6×6 Communication Matrix (30+ Paths)** — All permutations of 6 disability profiles; existing work covers at most 3
3. **🏆 Modular Ecosystem Architecture** — Specialized low-cost modules + shared hub; each user buys only what they need ($15–90 vs. $300+ monolithic)
4. **🏆 Tremor-Tolerant Gaze Keyboard** — Low-cost ($50–65) vs. Tobii Dynavox ($5000+); Gaussian tremor filter + adaptive dwell-time
5. **🏆 Hybrid Sensor-Vision Fusion** — Glove + MediaPipe on module captures all 5 sign language parameters; zero extra hardware; no data leaves module until TEXT

---

## 6. HARDWARE BOM

### Per-Module Components

**Glove Module (Deaf/Mute):**
| Component | Purpose |
|---|---|
| ESP32-S3 WROOM | Glove MCU |
| 5× Spectra Symbol Flex Sensors (2.2") | Finger bend |
| ADS1115 16-bit ADC | High-res analog |
| BNO085 9-DOF IMU | Palm orientation |
| MAX30102 + MPU6050 | Health monitoring + fall detection |
| 3.7V LiPo + TP4056 | Glove power |
| RPi Zero 2W | Camera ML processing |
| RPi Camera Module (Standard) | MediaPipe vision |
| Small LCD Screen (3.5–5") | Avatar/text display |

**Braille Module (Blind):**
| Component | Purpose |
|---|---|
| ESP32-S3 WROOM | Module MCU |
| 6× Tactile Buttons + 1 Spacebar | Braille input (Perkins) |
| 2× Micro Servo Motors (SG90/MG90S) | Left servo → left rack (dots 1,2,3); Right servo → right rack (dots 4,5,6) |
| 2× Rack Bars with stepped cam profile (3D printed) | 8 positions per rack → selectively raises 3 pins per column |
| 2× Pinion Gears (3D printed) | Converts servo rotation → linear rack motion |
| 6× Metallic Pin Rods (2mm) | Braille dot pins raised/lowered by rack cam steps |
| 3D Printed Pin Housing + Frame | Holds pins in 2×3 layout + structural mount |
| Speaker + MAX98357A Amp | TTS audio |
| LiPo + TP4056 | Power |

**Gaze Module (Paralyzed/Parkinson's):**
| Component | Purpose |
|---|---|
| RPi Zero 2W | Eye-tracking processing |
| RPi Camera Module 3 NoIR (Wide) | Eye capture in all lighting |
| 850nm IR LED Ring | Low-light illumination |
| Small LCD Screen (3.5–5") | Gaze keyboard UI / text |
| Speaker + MAX98357A Amp | TTS audio |
| USB-C Power Bank | Power |

**Voice Module (Hearing):**
| Component | Purpose |
|---|---|
| ESP32-S3 WROOM | Module MCU |
| INMP441 I2S Mic | Voice capture |
| Speaker + MAX98357A Amp | Audio output |

**Central Hub:**
| Component | Purpose |
|---|---|
| Raspberry Pi 5 (8GB) + Active Cooler | Central processing |
| M.2 NVMe HAT + 256GB SSD | Storage |
| 7" DSI Touchscreen | Admin UI |
| 27W USB-C PD Power Supply | Power (always-on) |

---

## 7. SOFTWARE STACK

| Component | Language | Key Libraries |
|---|---|---|
| ESP32-S3 firmware | C/C++ (Arduino/ESP-IDF) | Kalman filter, WiFi MQTT, I2C |
| RPi Zero 2W (Glove) | Python | MediaPipe Holistic, TFLite Bi-LSTM, Paho MQTT |
| RPi Zero 2W (Gaze) | Python | MediaPipe Iris, Gaussian filter, Paho MQTT |
| Central Hub (RPi 5) | Python | Vosk/Whisper (STT), Piper (TTS), Mosquitto, PyCryptodome, Flask/PyQt |
| Cloud | — | Mosquitto v5.0, TLS 1.3 |

---

## 8. AMP PROTOCOL (Ability Message Protocol)

```json
{
  "amp_version": "1.0",
  "message_id": "uuid-v4",
  "sender_hub_id": "hub_dhaka_01",
  "sender_module_id": "glove_rahim_01",
  "sender_profile": "deaf",
  "target_hub_id": "hub_berlin_01",
  "target_module_id": "braille_fatima_01",
  "target_profile": "blind",
  "content_type": "text",
  "content": "Hello, how are you?",
  "language": "en",
  "timestamp_utc": "2026-08-17T14:30:00Z",
  "encrypted": true,
  "encryption_algo": "AES-256-GCM",
  "qos": 2,
  "ttl_seconds": 86400
}
```

**Local MQTT Topics (Module ↔ Hub):**
- `ability/local/{module_id}/input` — module sends TEXT/audio
- `ability/local/{module_id}/output` — hub sends converted output

**Remote MQTT Topics (Hub ↔ Cloud ↔ Hub):**
- `ability/remote/{target_hub_id}/inbox` — encrypted AMP messages

---

## 9. TARGET PUBLICATION

| Attribute | Detail |
|---|---|
| **Primary Target** | IEEE Internet of Things Journal (IF 8.2, Q1) |
| **Backup Targets** | IEEE Sensors Journal (Q1), Sensors MDPI (Q1/Q2), IEEE Access (Q1) |
| **Paper Framing** | Novel modular cross-disability remote communication ecosystem — NOT "smart glove" |
| **Proposed Title** | "Ability: A Modular IoT Ecosystem for Cross-Disability Remote Communication with Multi-Modal Sensory Translation and Tremor-Tolerant Gaze Input" |

---

## 10. KEY DOCUMENTS INDEX

| File | What It Contains |
|---|---|
| `PROJECT_BRAIN.md` | **THIS FILE** — master reference + TODO tracker |
| `plan/MASTER_PLAN.md` | Full research plan — all 30 paths, per-module BOM, roadmap, paper structure |
| `.agents/rules/project-context.md` | Auto-loading AI context rule |
| `Ability_Project_Proposal.pdf` | Original project proposal |
| `anti/novelty_analysis.md` | Literature review + Q1 publishability verdict |
| `anti/project_ability_slide_deck.md` | 23-slide course presentation guide |
| `anti/research_proposal_slides.md` | 11-slide Q1 journal pitch |
| `anti/sign_language_fix.md` | Hybrid fusion technical deep-dive |

---

## 11. CURRENT STATUS & PROGRESS

### Phase Overview
| Phase | Description | Status |
|---|---|---|
| Phase 0 | Research & Proposal | ✅ Complete |
| Phase 1 | Hardware Assembly & Bring-Up | ⬜ Not Started |
| Phase 2 | Module Firmware & Input Encoders | ⬜ Not Started |
| Phase 3 | Central Hub Software | ⬜ Not Started |
| Phase 4 | Cloud & Remote Testing | ⬜ Not Started |
| Phase 5 | User Study & Paper Writing | ⬜ Not Started |

### What's Been Done ✅
- [x] Project concept and scope defined
- [x] Literature review of 40+ papers (2024-2025)
- [x] Novelty analysis — 5 contributions identified
- [x] Modular ecosystem architecture designed (4 modules + hub + cloud)
- [x] Per-module BOM finalized
- [x] Hybrid sensor-vision fusion approach designed
- [x] Gaze keyboard approach designed (tremor-tolerant)
- [x] Communication matrix (6×6, 30+ paths) defined
- [x] AMP protocol specified (local + remote MQTT)
- [x] Security model designed (E2EE, AES-256-GCM, TLS 1.3)
- [x] Course proposal slide deck created (23 slides)
- [x] Research proposal slide deck created (11 slides)
- [x] Target journals identified and ranked

---

## 12. MASTER TODO

### 🔴 CRITICAL (Must-Have Before Paper Submission)
- [ ] **Build all 4 module types** — Glove, Braille, Gaze, Voice modules physically working
- [ ] **Build Central Hub** — RPi 5 with local MQTT broker + all converters running
- [ ] **Train hybrid fusion ML** — Bi-LSTM on Glove Module (target >90% accuracy)
- [ ] **Measure real latency** — per-path, broken down by stage (module → hub → network → module)
- [ ] **Demo remote communication** — two hubs communicating over real internet
- [ ] **At least one cross-disability path working** — e.g., Sign → Remote Braille
- [ ] **User study** — 5+ participants across disability profiles
- [ ] **Comparison table** — Ability vs. 3-5 existing systems
- [ ] **Per-module power consumption + cost data**
- [ ] **Ethics/IRB approval**

### 🟡 IMPORTANT (Strengthens Paper)
- [ ] Multiple gesture vocabularies tested (ASL alphabet, common phrases)
- [ ] Network resilience testing (latency, packet loss scenarios)
- [ ] Braille reading speed with blind users
- [ ] Avatar comprehension with deaf users
- [ ] Gaze keyboard WPM + error rate with Parkinson's users
- [ ] Cost comparison vs. Tobii ($5000+), commercial Braille displays ($3000+)
- [ ] Open-source release of dataset + BOM

### 🟢 NICE-TO-HAVE (Future Work)
- [ ] Web/mobile client companion
- [ ] Multi-language sign support (ASL, BSL, ISL)
- [ ] Voice clone / personalized TTS
- [ ] Predictive text for gaze keyboard

---

## 13. DESIGN DECISIONS & CONSTRAINTS

### Do's ✅
- Frame as **modular cross-disability remote communication ecosystem**
- Emphasize **bidirectional** — every user talks to every other user
- Emphasize **global** — works from anywhere in the world
- Emphasize **modular** — each user buys only what they need ($15–90)
- Include all **6 profiles** — especially Parkinson's (key differentiator)
- Keep ML **on modules** (privacy — no raw data leaves the module)
- Use **TEXT as universal intermediate** — simplifies O(n²) to O(n)

### Don'ts ❌
- Don't call it a single-disability device or "smart glove"
- Don't submit concept-only paper — Q1 requires prototype + experiments
- Don't stream video — only TEXT/audio/commands travel over the network
- Don't put all hardware on one device — modular ecosystem means specialized modules
- Don't ignore the 5-parameter sign language problem
- Don't skip user studies — assistive tech papers require them

### Key Technical Decisions
- **Modular (not monolithic)** — each user gets only what they need; cheaper, scalable
- **ESP32 for simple modules** — sufficient for buttons, mic, servo PWM; low power, cheap
- **RPi Zero 2W for vision modules** — cheapest Linux board with camera + MediaPipe capability
- **RPi 5 for Central Hub** — powerful enough for STT/TTS, format conversion, routing
- **MQTT locally + remotely** — consistent pub/sub pattern everywhere
- **Audio streaming (not local STT) for Voice Module** — 32kbps is negligible; keeps module cheap
- **Pre-stored avatar clips (not real-time 3D)** — Zero 2W can't render 3D; hub sends sign IDs
- **BNO085 IMU (not MPU6050 for orientation)** — drift-free; MPU6050 only for fall detection
- **Bi-LSTM (not Transformer)** — lighter for RPi Zero 2W; sufficient for sign classification
- **Late fusion** — different sampling rates; preserves modality-specific features
- **AES-256-GCM + TLS 1.3** — E2EE so cloud never sees plaintext

---

## 14. REAL-WORLD USE CASES

1. **Family Home** — Hub in living room. Deaf son signs "dinner is ready" via Glove Module → Hub converts → Mother's Braille Module actuates dots. Mother types back "coming" → Son sees avatar.

2. **Cross-Country** — Deaf grandson (Dhaka) signs "I miss you" → Hub A → MQTT cloud → Hub B (Sylhet) → Blind grandmother's Braille Module: ⠠⠊ ⠍⠊⠎⠎ ⠽⠕⠥

3. **Hospital** — Paralyzed patient eye-types "chest pain" via Gaze Module → Hub → Doctor's Voice Module plays TTS. Doctor speaks back → Hub STT → Patient reads on screen.

4. **Parkinson's Independence** — Mr. Karim gaze-types "Happy birthday" → Hub → MQTT → Deaf granddaughter's avatar. She signs back → TTS plays for Mr. Karim.

---

## 15. QUICK REFERENCE NUMBERS

| Metric | Value |
|---|---|
| Disability profiles | 6 |
| Cross-disability paths | 30+ (all bidirectional, all remote-capable) |
| Module types | 4 (Glove, Braille, Gaze, Voice) |
| Input encoders | 6 (sign, Braille, gesture, gaze cursor, gaze KB, voice) |
| Output renderers | 4 (avatar, Braille gear pins, TTS, screen text) |
| Module cost range | $15–90 (vs. $300+ monolithic) |
| Central Hub cost | ~$150–200 (shared, not per-user) |
| Sign language parameters | 5/5 (via hybrid fusion on module) |
| MediaPipe landmarks | 543 (33 pose + 42 hand + 468 face) |
| Feature vector | ~1600 per frame |
| Target latency | <500ms end-to-end |
| Target accuracy | >90% (aim >95%) |
| Audio streaming bandwidth | 32 kbps (Voice Module → Hub) |
| Encryption | AES-256-GCM + TLS 1.3 |
| MQTT QoS | 2 (exactly-once delivery) |
| Global reach | Anywhere with internet via MQTT relay |
| People affected globally | 800M+ |

---

> **⚠️ This file is the quick reference. For full details (all 30 paths explained step-by-step, per-module BOM, implementation roadmap, paper structure), see `plan/MASTER_PLAN.md`.**
