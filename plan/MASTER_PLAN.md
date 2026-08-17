# Project Ability — Research & Implementation Master Plan

> **Document Type:** Research-Level Implementation Roadmap
> **Version:** 2.0 (Modular Ecosystem Architecture)
> **Created:** 2026-08-17
> **Last Updated:** 2026-08-17
> **Status:** Active

---

## 1. PROJECT IDENTITY

### 1.1 What Project Ability Is

Project Ability is a **research-level, microprocessor-based cross-disability communication ecosystem**. It is a **modular platform** where each disabled person owns a **personal, disability-specific module** (only the hardware they need), and all modules connect to a **shared Central Hub** that handles format conversion, translation routing, and global connectivity.

The system works like a **universal translator**: each user interacts using the modality natural to them (sign language, Braille, eye-gaze, voice), and the platform **automatically translates** that input into the output format that the receiver needs (physical Braille pins, 3D signing avatar, synthesized speech, on-screen text).

### 1.2 The Core Principle — Everyone Talks to Everyone

This is **NOT** a one-way assistive device. It is a **bidirectional, cross-disability, globally connected communication ecosystem**. The fundamental design principle:

> **Any person with any disability can send a message to any other person with any other disability, anywhere in the world, and the system translates it into the receiver's native format automatically.**

- A **deaf** person signs → a **blind** person feels Braille
- That **blind** person types Braille back → the **deaf** person sees a 3D avatar signing the reply
- A **Parkinson's** patient gazes at a keyboard → their **deaf** grandchild sees a signing avatar
- That **deaf** grandchild signs back → the **Parkinson's** patient hears it spoken aloud
- A **paralyzed** person blinks to type → a **mute** person reads it on screen
- A **hearing** person speaks → a **blind** person feels the words in Braille

**Every combination works. Every direction works. Distance is irrelevant.**

### 1.3 The Ecosystem Philosophy

Project Ability is **not a single monolithic device** — it is an **ecosystem of specialized modules**:

- Each disabled person buys or builds **only the module for their disability** — nothing more
- They don't pay for hardware they will never use (a blind person has no camera or screen)
- All modules connect to a **shared Central Hub** (like a router for disability communication)
- The Central Hub does the format conversion, routing, and internet connectivity
- For global reach, Central Hubs connect to each other via an encrypted cloud relay

> *"Buy your module. Join the network. Talk to anyone."*

---

## 2. THE 6 USER PROFILES & THEIR MODALITIES

Each disability profile has specific **input** (how they send) and **output** (how they receive) modalities that match their physical capabilities:

| # | Disability Profile | Why They Can't Use Normal Communication | Input Modality (How They SEND) | Output Modality (How They RECEIVE) |
|---|---|---|---|---|
| 1 | **Deaf / Hard of Hearing** | Cannot hear speech; uses visual-spatial language | **Sign Language** via Sensor Glove + Camera (Hybrid Fusion) | **3D Sign Language Avatar** on screen |
| 2 | **Blind / Low Vision** | Cannot see screens, text, or avatars | **Braille Input** via 6 tactile buttons (Perkins layout) | **Braille Output** via rack-and-pinion gear-driven pins (physical dots) + **TTS** (spoken audio) |
| 3 | **Mute / Speech Impaired** | Cannot produce speech; can see and move hands | **Gestures** via Sensor Glove + Camera | **Text on Screen** (can read normally) |
| 4 | **Paralyzed (ALS, Spinal Cord Injury)** | Cannot move hands or body; eyes still work | **Eye-Gaze Cursor** + Blink-to-Click | **Text on Screen** + **TTS** (spoken audio) |
| 5 | **Parkinson's / Neurodegenerative** | Hands tremble too much for keyboards; fine motor control lost | **Gaze Keyboard** with tremor-tolerant filtering | **Text on Screen** + **TTS** (spoken audio) |
| 6 | **Hearing / Sighted (General Public)** | No disability — bridges the gap to the disabled community | **Voice** via microphone (STT) | **Text on Screen** + **Speech** (can use any format) |

---

## 3. THE COMPLETE 6×6 CROSS-DISABILITY COMMUNICATION MATRIX

This is the heart of the project. Every cell below represents a **real, working translation pipeline** — each one a specific sequence of: capture → translate on module → send TEXT to hub → hub converts to receiver's format → deliver to receiver's module.

### 3.1 Full Matrix (30 Unique Cross-Disability Paths)

| SENDER ↓ \ RECEIVER → | 🦻 **Deaf** (gets Avatar) | 👁️ **Blind** (gets Braille/TTS) | 🤐 **Mute** (gets Screen Text) | 🧑‍🦽 **Paralyzed** (gets Screen Text/TTS) | 🧠 **Parkinson's** (gets Screen Text/TTS) | 👂 **Hearing** (gets Speech/Text) |
|---|---|---|---|---|---|---|
| 🦻 **Deaf** (sends Sign) | — | Sign → Text → **Braille** | Sign → **Text** | Sign → **Text/TTS** | Sign → **Text/TTS** | Sign → **Speech (TTS)** |
| 👁️ **Blind** (sends Braille) | Braille → Text → **Avatar** | — | Braille → **Text** | Braille → **Text/TTS** | Braille → **Text/TTS** | Braille → **Speech (TTS)** |
| 🤐 **Mute** (sends Gesture) | Gesture → Text → **Avatar** | Gesture → Text → **Braille** | — | Gesture → **Text/TTS** | Gesture → **Text/TTS** | Gesture → **Speech (TTS)** |
| 🧑‍🦽 **Paralyzed** (sends Gaze) | Gaze → Text → **Avatar** | Gaze → Text → **Braille** | Gaze → **Text** | — | Gaze → **Text/TTS** | Gaze → **Speech (TTS)** |
| 🧠 **Parkinson's** (sends Gaze KB) | GazeKB → Text → **Avatar** | GazeKB → Text → **Braille** | GazeKB → **Text** | GazeKB → **Text/TTS** | — | GazeKB → **Speech (TTS)** |
| 👂 **Hearing** (sends Voice) | Voice → Text → **Avatar** | Voice → Text → **Braille** | Voice → **Text** | Voice → **Text/TTS** | Voice → **Text/TTS** | — |

> **All 30 paths work both locally (same hub) AND remotely (across hubs, anywhere in the world via encrypted MQTT).**

### 3.2 The Universal Intermediate — TEXT

The architectural key that makes 30+ paths possible without building 30 separate translators:

```
                    ┌─────────────────────────┐
                    │                         │
  [Sign Language] ──┤                         ├──► [3D Avatar]
  [Braille Input] ──┤                         ├──► [Braille Gear Pins]
  [Gestures]      ──┤    TEXT (Universal      ├──► [TTS Speech]
  [Eye-Gaze]      ──┤    Intermediate)        ├──► [Screen Text]
  [Gaze Keyboard] ──┤                         │
  [Voice / STT]   ──┤                         │
                    └─────────────────────────┘

  6 Input Encoders        TEXT         4 Output Renderers
  (on individual modules)              (on Central Hub,
                                        delivered to modules)
```

**Every input modality converts TO text (on the module). Every output modality converts FROM text (on the Central Hub, then delivered to the target module).** This means:
- Adding a new input = 1 new module design (automatically reaches all outputs)
- Adding a new output = 1 new renderer on the hub (automatically reached by all inputs)
- Total translators needed: 6 encoders + 4 renderers = **10** (not 30)

---

## 4. MODULAR ECOSYSTEM ARCHITECTURE

### 4.1 Architecture Overview — Three Tiers (Redefined)

```
  ┌─────────────────────────────────────────────────────────────────────────────────┐
  │  TIER 1: INDIVIDUAL MODULES (Personal, Disability-Specific, Cheap)             │
  │                                                                                 │
  │  ┌──────────────┐ ┌──────────────┐ ┌──────────────┐ ┌──────────────┐           │
  │  │ Glove Module │ │ Braille Mod  │ │  Gaze Module │ │ Voice Module │           │
  │  │ (Deaf/Mute)  │ │ (Blind)      │ │ (Paralyzed/  │ │ (Hearing)    │           │
  │  │              │ │              │ │  Parkinson's) │ │              │           │
  │  │ ESP32+Glove  │ │ ESP32        │ │ RPi Zero 2W  │ │ ESP32+Mic   │           │
  │  │ +RPiZero+Cam │ │ +Buttons     │ │ +NoIR Camera │ │ +Speaker     │           │
  │  │ +Small Screen│ │ +Gear Pins   │ │ +IR LEDs     │ │              │           │
  │  │              │ │ +Speaker     │ │ +Small Screen│ │              │           │
  │  │ Sends: TEXT  │ │ Sends: TEXT  │ │ Sends: TEXT  │ │ Sends: Audio │           │
  │  └──────┬───────┘ └──────┬───────┘ └──────┬───────┘ └──────┬───────┘           │
  │         │ WiFi           │ WiFi           │ WiFi           │ WiFi              │
  └─────────┼────────────────┼────────────────┼────────────────┼───────────────────┘
            │                │                │                │
            ▼                ▼                ▼                ▼
  ┌─────────────────────────────────────────────────────────────────────────────────┐
  │  TIER 2: CENTRAL HUB (Shared per household/classroom/clinic)                   │
  │                                                                                 │
  │  Raspberry Pi 5 (8GB) + NVMe SSD + 7" Admin Touchscreen                       │
  │                                                                                 │
  │  ┌────────────────────────────────────────────────────────────────────────┐     │
  │  │  RESPONSIBILITIES:                                                     │     │
  │  │  • Receives TEXT from all modules (or audio from Voice Module → STT)  │     │
  │  │  • FORMAT CONVERSION: Text → Braille / Avatar commands / TTS / Text   │     │
  │  │  • LOCAL ROUTING: Module A → Hub → Module B (same network)            │     │
  │  │  • REMOTE ROUTING: Encrypt → MQTT → Cloud → Remote Hub               │     │
  │  │  • User profile management & contact list                             │     │
  │  │  • Conversation history & health monitoring                           │     │
  │  │  • Local MQTT broker for module communication                         │     │
  │  │  • E2EE encryption/decryption for remote messages                     │     │
  │  └────────────────────────────────────────────────────────────────────────┘     │
  │                                                                                 │
  └─────────────────────────────────────┬───────────────────────────────────────────┘
                                        │ Internet (WiFi / Ethernet / 4G)
                                        ▼
  ┌─────────────────────────────────────────────────────────────────────────────────┐
  │  TIER 3: CLOUD RELAY (Global Message Routing)                                   │
  │                                                                                 │
  │  Mosquitto MQTT Broker on VPS                                                   │
  │  • Routes ENCRYPTED text between Central Hubs worldwide                        │
  │  • Never reads content (E2EE — only endpoints can decrypt)                     │
  │  • TLS 1.3 transport + AES-256-GCM payload encryption                          │
  │  • QoS Level 2 (exactly-once delivery)                                         │
  │  • Offline message queuing (delivers when hub reconnects)                      │
  │                                                                                 │
  └─────────────────────────────────────────────────────────────────────────────────┘
```

### 4.2 The 4 Module Types (Tier 1)

Only **4 distinct hardware designs** serve all **6 disability profiles**:

#### Module Type 1: GLOVE MODULE (for Deaf & Mute users)

| Component | Spec | Purpose |
|---|---|---|
| ESP32-S3 WROOM | Dual-core LX7 @ 240 MHz, BLE+WiFi | Glove MCU — reads sensors, streams to local RPi Zero |
| 5× Spectra Symbol Flex Sensors (2.2") | Analog | Finger bend detection |
| ADS1115 | 16-bit, 4-channel, I²C | High-resolution ADC for flex sensors |
| BNO085 9-DOF IMU | Sensor fusion, drift-free | Palm orientation tracking |
| MAX30102 | Pulse oximetry | SpO2 + heart rate (health monitoring) |
| MPU6050 | Accelerometer + gyroscope | Fall detection |
| 3.7V 1000mAh LiPo + TP4056 | — | Glove power + charging |
| RPi Zero 2W | Quad-core Cortex-A53 @ 1GHz, 512MB | Local camera processing (MediaPipe) |
| RPi Camera Module (Standard) | CSI, via adapter cable | Captures body pose + hand location + facial expressions |
| Small LCD Screen (3.5–5") | SPI/HDMI | Deaf: avatar playback / Mute: text display |
| 28 AWG silicone-jacketed wire | — | Wiring |

**How it works:**
1. ESP32 reads glove sensors (flex + IMU + health) at 100+ Hz
2. ESP32 sends sensor data to local RPi Zero 2W via serial/I2C (wired, within the same module)
3. RPi Zero 2W simultaneously captures camera frames → runs MediaPipe Holistic (543 landmarks)
4. RPi Zero 2W **fuses** glove data + camera landmarks → Bi-LSTM model → **TEXT**
5. RPi Zero 2W sends **TEXT** to Central Hub via WiFi (tiny payload, ~100 bytes)
6. When receiving: Hub sends avatar commands (sign IDs) → RPi Zero 2W plays pre-stored animation clips on screen

**Deaf vs. Mute software modes:**
- Deaf mode: Sign language recognition → TEXT output; receives avatar animation for display
- Mute mode: Gesture recognition → TEXT output; receives plain text for display

**Estimated cost:** ~$70–90

---

#### Module Type 2: BRAILLE MODULE (for Blind users)

| Component | Spec | Purpose |
|---|---|---|
| ESP32-S3 WROOM | Dual-core, BLE+WiFi | Module MCU |
| 6× Tactile Push Buttons | Perkins layout | Braille character input |
| 1× Spacebar Button | — | Word/character separator |
| 2× Micro Servo Motors (SG90 or MG90S) | 4.8–6V, 180° | Left servo drives left rack; Right servo drives right rack |
| 2× Pinion Gears | 3D printed (PLA/ABS) | Meshes with rack; converts servo rotation → linear motion |
| 2× Rack Bars (with stepped cam profile) | 3D printed (PLA/ABS) | Each rack has 8 step positions to selectively raise 3 pins |
| 6× Metallic Pin Rods (2mm diameter) | Stainless steel | Braille dot pins — raised/lowered by rack cam steps |
| 3D Printed Pin Guide Housing | PLA/ABS | Holds 6 pins in standard Braille cell layout (2×3 grid) |
| 3D Printed Frame/Mount | PLA/ABS | Structural frame for servos + gear assembly |
| Small Speaker (3W) | — | TTS audio output |
| MAX98357A I2S Amp | — | Audio amplifier for speaker |
| 3.7V 1500mAh LiPo + TP4056 | — | Power + charging |

**Braille Output Mechanism — 2-Servo Rack and Pinion System:**

```
  BRAILLE CELL (2 columns × 3 rows = 6 dots)

  LEFT COLUMN        RIGHT COLUMN
  [Dot 1] [Dot 4]
  [Dot 2] [Dot 5]
  [Dot 3] [Dot 6]

  ┌─────────────────────────────┐  ┌─────────────────────────────┐
  │  LEFT SERVO                 │  │  RIGHT SERVO                │
  │       │                     │  │       │                     │
  │       ▼                     │  │       ▼                     │
  │  Left Pinion Gear           │  │  Right Pinion Gear          │
  │       │                     │  │       │                     │
  │       ▼                     │  │       ▼                     │
  │  Left Rack Bar              │  │  Right Rack Bar             │
  │  (stepped cam profile)      │  │  (stepped cam profile)      │
  │       │                     │  │       │                     │
  │       ▼                     │  │       ▼                     │
  │  Selectively raises:        │  │  Selectively raises:        │
  │  Dot 1, Dot 2, Dot 3       │  │  Dot 4, Dot 5, Dot 6       │
  │  (8 positions = 8 combos)  │  │  (8 positions = 8 combos)  │
  └─────────────────────────────┘  └─────────────────────────────┘

  Left Servo (8 positions)  ×  Right Servo (8 positions)  =  64 patterns
                             ALL 6-DOT BRAILLE CHARACTERS ✓
```

**How the stepped cam profile works:**

Each rack bar has a **stepped/cam surface** machined or 3D-printed with 8 distinct height profiles. As the servo rotates the pinion to different angles, the rack slides to different positions. At each position, the cam steps under the 3 pin rods are at different heights — selectively pushing specific pins UP while leaving others DOWN.

```
  Example — Left Rack Positions (3 dots, 8 combinations):

  Position 0 (0°):    ▽▽▽  → No dots raised     (binary: 000)
  Position 1 (22°):   ▲▽▽  → Dot 1 only          (binary: 001)
  Position 2 (44°):   ▽▲▽  → Dot 2 only          (binary: 010)
  Position 3 (66°):   ▲▲▽  → Dots 1,2            (binary: 011)
  Position 4 (88°):   ▽▽▲  → Dot 3 only          (binary: 100)
  Position 5 (110°):  ▲▽▲  → Dots 1,3            (binary: 101)
  Position 6 (132°):  ▽▲▲  → Dots 2,3            (binary: 110)
  Position 7 (154°):  ▲▲▲  → All three dots       (binary: 111)

  Each step = ~22° of servo rotation (180° / 8 = 22.5°)
  SG90 servo precision: ~1° — more than sufficient for 8 positions
```

**How it works:**
1. Blind user presses Braille buttons (6 keys in Perkins layout = one character)
2. ESP32 reads button combination → decodes Braille pattern → **TEXT** character
3. User types full message character-by-character (with spacebar for spaces)
4. ESP32 sends completed **TEXT** message to Central Hub via WiFi
5. When receiving: Hub sends Braille byte encoding → ESP32 splits the 6-bit pattern into left 3 bits (dots 1,2,3) and right 3 bits (dots 4,5,6) → commands left servo to position X and right servo to position Y → racks slide via pinion gears → cam profiles raise the correct pins
6. Each character is held for ~1.5 seconds, then both servos return to position 0 (all pins retract) before the next character
7. Optionally: Hub sends TTS audio stream → ESP32 plays on speaker via I2S

**Why 2-Servo Rack-and-Pinion instead of Solenoids?**

| Aspect | Solenoids (Rejected) | 2-Servo Rack & Pinion (Chosen) |
|---|---|---|
| **Parts count** | 6 solenoids + ULN2003 + 6 diodes = 13 parts | 2 servos + 2 racks + 2 pinions = 6 parts |
| **GPIO pins needed** | 6 GPIO outputs (one per solenoid) | 2 GPIO PWM (one per servo) |
| **Driver circuit** | ULN2003 Darlington array + flyback diodes | None — ESP32 drives servos directly |
| **Noise** | Loud clicking on each actuation | Quiet gear operation |
| **Tactile quality** | Harsh binary snap up/down | Smooth, controlled raise/lower |
| **Power** | ~150–500mA per solenoid × 6 | ~100mA per servo × 2 (only when moving) |
| **Heat** | Solenoids generate heat during hold | Servo holds position with negligible heat |
| **Cost** | ~$15–20 for 6 solenoids + driver | ~$4–6 for 2 micro servos |
| **3D printing** | Not required | Rack bars + pinions + housing (custom, novel) |
| **Research novelty** | Solenoid Braille = well-published | **2-servo cam-profiled rack = novel mechanical approach** |

**No camera. No screen. No ML. Just buttons, 2 servos, gear-driven pins, and a speaker.** Simplest and cheapest module.

**Estimated cost:** ~$20–30

---

#### Module Type 3: GAZE MODULE (for Paralyzed & Parkinson's users)

| Component | Spec | Purpose |
|---|---|---|
| RPi Zero 2W | Quad-core Cortex-A53, 512MB | Eye-tracking processing (MediaPipe Iris) |
| RPi Camera Module 3 NoIR (Wide) | No IR filter, CSI via adapter | Captures eye/iris in all lighting conditions |
| 850nm IR LED Ring Module | Invisible infrared | Illuminates eyes in low-light for reliable tracking |
| Small LCD Screen (3.5–5") | SPI/HDMI | Gaze keyboard UI / text display |
| Small Speaker (3W) + MAX98357A | — | TTS audio output |
| USB-C Power Bank (10W) | — | Module power |

**How it works:**
1. Camera captures user's face → RPi Zero 2W runs MediaPipe Iris → extracts gaze coordinates
2. **Paralyzed mode:** Gaze coordinates map to screen cursor; Eye Aspect Ratio (EAR) detects blink = "click"; user selects on-screen characters → **TEXT**
3. **Parkinson's mode:** Gaze coordinates → Gaussian smoothing (tremor filter) → dwell-click on QWERTY keyboard (user stares at a key for 400–1500ms to "press" it) → word prediction assists → **TEXT**
4. RPi Zero 2W sends **TEXT** to Central Hub via WiFi
5. When receiving: Hub sends text → displayed on screen; Hub sends TTS audio → plays on speaker

**Paralyzed vs. Parkinson's software modes:**
- Paralyzed: Gaze cursor + blink click (no tremor filtering needed)
- Parkinson's: Gaze keyboard with Gaussian tremor smoothing + adaptive dwell time (400–1500ms) + word prediction

**Estimated cost:** ~$50–65

---

#### Module Type 4: VOICE MODULE (for Hearing/Sighted users)

| Component | Spec | Purpose |
|---|---|---|
| ESP32-S3 WROOM | Dual-core, BLE+WiFi | Module MCU |
| INMP441 I2S MEMS Mic | Omnidirectional | Voice capture |
| Small Speaker (3W) + MAX98357A | — | Audio output |
| Optional: Small OLED screen (1.3") | I²C | Status display / text preview |

**How it works:**
1. Hearing user speaks into microphone
2. ESP32 captures audio → streams to Central Hub via WiFi (16kHz mono = ~32 kbps — negligible bandwidth, NOT video streaming)
3. Central Hub runs offline STT (Vosk/Whisper) → converts speech → **TEXT**
4. Hub routes TEXT to target receiver's module
5. When receiving: Hub sends TTS audio → ESP32 plays on speaker

**Why stream audio instead of local STT?** STT engines (Vosk/Whisper) require more compute than ESP32 can handle. Audio streaming at 32 kbps over WiFi is trivially lightweight — less than a phone call. The Central Hub has the RPi 5 to run STT efficiently.

**Estimated cost:** ~$15–25

---

### 4.3 Central Hub (Tier 2) — The Brain of the Ecosystem

The Central Hub is a **shared processing and routing unit** — like a router for disability communication. One hub serves an entire household, classroom, or clinic. It contains **no sensors and no actuators** — those are all on individual modules.

| Component | Spec | Purpose |
|---|---|---|
| Raspberry Pi 5 | 8GB RAM, Cortex-A76 @ 2.4 GHz | Central processing |
| Active Cooler | Fan + heatsink | Thermal management |
| M.2 NVMe HAT + 256GB SSD | — | Fast storage (conversation history, ML models, avatar clips) |
| 7" DSI Capacitive Touchscreen | — | Admin interface (user profiles, contact list, system status) |
| 27W USB-C PD Power Supply | — | Hub power (always-on) |
| Ethernet Cable (optional) | — | Reliable internet for MQTT |

**What the Central Hub does:**

| Responsibility | Details |
|---|---|
| **Receive TEXT** | From all connected modules via local WiFi/MQTT |
| **STT for Voice Module** | Receives raw audio (32 kbps) → Vosk/Whisper → TEXT |
| **Format Conversion** | TEXT → Braille encoding (for Blind modules) |
| | TEXT → Avatar sign sequence IDs (for Deaf modules) |
| | TEXT → TTS audio bytes via Piper (for modules with speakers) |
| | TEXT → display text string (for modules with screens) |
| **Local Routing** | Routes converted output to target module on the same network |
| **Remote Routing** | Encrypts TEXT (AES-256-GCM) → publishes to MQTT cloud → remote hub |
| **Remote Receiving** | Subscribes to MQTT → decrypts → converts to local module's format → delivers |
| **User Management** | Stores user profiles (name, disability type, paired module ID) |
| **Contact List** | Maps which remote users this hub's users can communicate with |
| **Conversation History** | Stores message logs locally on NVMe SSD |
| **Health Monitoring** | Receives SpO2/heart rate/fall data from Glove Modules → triggers alerts |
| **Local MQTT Broker** | Runs Mosquitto locally — all modules connect to it for local communication |
| **E2EE** | AES-256-GCM encryption for outgoing remote messages; decryption for incoming |

**What the Central Hub does NOT have:** No camera, no microphone, no speaker, no glove, no Braille buttons, no Braille actuators. It is a **pure processing and routing unit**.

**Estimated cost:** ~$150–200

---

### 4.4 Cloud Relay (Tier 3)

A lightweight MQTT broker on a VPS that **only routes encrypted messages** between remote Central Hubs. It never processes, reads, or stores message content.

| Component | Spec | Purpose |
|---|---|---|
| VPS | 1 vCPU, 1GB RAM (AWS/DigitalOcean/Vultr) | Hosts broker |
| Mosquitto MQTT Broker | v5.0 | Message routing |
| Let's Encrypt TLS Certificate | — | Transport encryption |

**Monthly cost:** ~$5–10/month

---

### 4.5 Why This Architecture Is Superior

| Aspect | Old Architecture (Full Hub Per User) | New Architecture (Modular Ecosystem) |
|---|---|---|
| **Cost per user** | ~$300+ (full RPi 5 + ALL peripherals) | ~$15–90 (only what their disability needs) |
| **Wasted hardware** | Blind user has camera+display they never use | Zero waste — each module is tailored |
| **Philosophy** | Everyone gets the same monolithic box | "Buy YOUR module, join the network" |
| **Scalability** | Adding new disability = redesign everything | Adding new disability = design one new module |
| **ML updates** | Update on every device separately | Update on Central Hub once, all users benefit |
| **Maintainability** | Each user's RPi 5 needs software maintenance | Only Central Hub needs maintenance |
| **Research novelty** | "We built a device" | "We designed a **modular disability ecosystem**" — much stronger |
| **Paper framing** | Another assistive device | A **platform architecture** — Q1 journals value this |

---

## 5. DATA FLOW — COMPLETE END-TO-END

### 5.1 Sending a Message (Any Module → Central Hub)

```
Step 1: INPUT CAPTURE (on individual module)
  Module's sensors capture raw data specific to the user's disability:
  • Glove Module: flex sensor voltages + IMU quaternions + camera frames
  • Braille Module: button press combinations
  • Gaze Module: camera frames of user's eyes
  • Voice Module: microphone audio stream

       │
       ▼

Step 2: LOCAL INPUT PROCESSING (on individual module)
  Module converts its raw input → TEXT using local processing:
  ┌──────────────────────────────────────────────────────────────────┐
  │ Glove Module:  Flex+IMU+Camera → MediaPipe+Bi-LSTM → TEXT      │
  │                (on RPi Zero 2W — hybrid fusion, no streaming)   │
  │                                                                  │
  │ Braille Module: Button combo → Braille decoder → TEXT           │
  │                 (on ESP32 — simple table lookup)                 │
  │                                                                  │
  │ Gaze Module:   Camera → MediaPipe Iris → Gaze/Dwell → TEXT     │
  │                (on RPi Zero 2W — local processing)              │
  │                                                                  │
  │ Voice Module:  Audio → streamed to Hub (32 kbps)                │
  │                Hub runs Vosk STT → TEXT                          │
  │                (only module that sends raw data — but audio,     │
  │                 NOT video; 32 kbps is negligible)                │
  └──────────────────────────────────────────────────────────────────┘

       │
       ▼

Step 3: SEND TEXT TO CENTRAL HUB (WiFi, tiny payload)
  Module publishes TEXT to local MQTT topic:
  Topic: ability/local/{module_id}/input
  Payload: {"text": "Hello, how are you?", "sender_profile": "deaf"}
  Size: ~100–200 bytes per message
```

### 5.2 Central Hub Processing & Routing

```
Step 4: HUB RECEIVES TEXT
  Central Hub subscribes to: ability/local/+/input
  Receives TEXT from any connected module.
  (For Voice Module: receives audio → runs STT → TEXT first)

       │
       ▼

Step 5: HUB DETERMINES TARGET & CONVERTS FORMAT
  Hub looks up: Who is this message for? What is their disability profile?
  Hub converts TEXT → target receiver's format:
  ┌──────────────────────────────────────────────────────────────────┐
  │ Target = Deaf:      TEXT → Sign sequence IDs ["HELLO","HOW",    │
  │                     "ARE","YOU","QUESTION"]  (~100 bytes)       │
  │                                                                  │
  │ Target = Blind:     TEXT → Braille byte encoding                │
  │                     [0x13,0x05,0x0C,0x0C,0x0F]  (~20 bytes)    │
  │                     + optional TTS audio                        │
  │                                                                  │
  │ Target = Mute:      TEXT → text string (pass-through)           │
  │                                                                  │
  │ Target = Paralyzed: TEXT → text string + optional TTS audio     │
  │                                                                  │
  │ Target = Parkinson's: TEXT → text string + optional TTS audio   │
  │                                                                  │
  │ Target = Hearing:   TEXT → TTS audio bytes                      │
  └──────────────────────────────────────────────────────────────────┘

       │
       ▼

Step 6: DELIVER TO TARGET MODULE
  ┌───────────────────────────────────────────────────────────────────┐
  │ IF target is LOCAL (connected to same hub):                      │
  │   Hub publishes converted data to: ability/local/{target_id}/out │
  │   Target module receives and renders output                      │
  │                                                                   │
  │ IF target is REMOTE (connected to a different hub):              │
  │   Hub encrypts TEXT with AES-256-GCM (receiver hub's public key) │
  │   Hub publishes to cloud MQTT: ability/remote/{target_hub}/inbox │
  │   Cloud broker routes to target hub                              │
  │   Target hub decrypts → converts to local module's format        │
  │   Target hub delivers to local module                            │
  └───────────────────────────────────────────────────────────────────┘
```

### 5.3 Output Rendering (on Individual Module)

```
Step 7: MODULE RECEIVES & RENDERS OUTPUT
  Each module renders the received data in its native format:
  ┌──────────────────────────────────────────────────────────────────┐
  │ Deaf Module:      Receives sign IDs → plays pre-stored          │
  │                   animation clips on small LCD screen            │
  │                                                                  │
  │ Blind Module:     Receives Braille bytes → ESP32 splits into    │
  │                   left/right columns → 2 servos drive racks     │
  │                   → cam profiles raise pins (char by char)       │
  │                   + receives TTS audio → plays on speaker        │
  │                                                                  │
  │ Mute Module:      Receives text string → displays on LCD screen  │
  │                                                                  │
  │ Paralyzed Module: Receives text → displays on LCD screen         │
  │                   + receives TTS audio → plays on speaker        │
  │                                                                  │
  │ Parkinson's Mod:  Receives text → displays on LCD screen         │
  │                   + receives TTS audio → plays on speaker        │
  │                                                                  │
  │ Voice Module:     Receives TTS audio → plays on speaker          │
  └──────────────────────────────────────────────────────────────────┘
```

### 5.4 What Travels Over the Network

**Between modules and Central Hub (local WiFi) — ALL LIGHTWEIGHT:**

| Data Type | Direction | Size | Example |
|---|---|---|---|
| TEXT message | Module → Hub | ~100 bytes | `"Hello, how are you?"` |
| Audio stream | Voice Module → Hub | 32 kbps | 16kHz mono PCM (less than a phone call) |
| Braille encoding | Hub → Blind Module | ~20 bytes | `[0x13, 0x05, 0x0C, 0x0C, 0x0F]` |
| Avatar commands | Hub → Deaf Module | ~100 bytes | `["HELLO", "HOW", "ARE", "YOU"]` |
| TTS audio | Hub → Any Module | ~50 kbps | Compressed audio clip |
| Display text | Hub → Screen Modules | ~100 bytes | `"Hello, how are you?"` |
| Health data | Glove Module → Hub | ~50 bytes | `{"spo2": 98, "hr": 72, "fall": false}` |

**Zero video streaming. Zero raw sensor data over the network. Only tiny, processed payloads.**

---

## 6. DETAILED TRANSLATION PIPELINES (Every Path)

### Path Group A: DEAF SENDER (Glove Module → TEXT)

| # | Path | Full Pipeline |
|---|---|---|
| A1 | Deaf → Blind | Glove+Camera on module → Hybrid Fusion → **TEXT** → WiFi to Hub → Hub converts TEXT → Braille encoding → WiFi to Blind Module → **Gear pins actuate** |
| A2 | Deaf → Mute | Glove+Camera → **TEXT** → Hub → TEXT string → Mute Module → **Display on screen** |
| A3 | Deaf → Paralyzed | Glove+Camera → **TEXT** → Hub → TEXT + TTS audio → Paralyzed Module → **Screen + Speaker** |
| A4 | Deaf → Parkinson's | Glove+Camera → **TEXT** → Hub → TEXT + TTS audio → Parkinson's Module → **Screen + Speaker** |
| A5 | Deaf → Hearing | Glove+Camera → **TEXT** → Hub → TTS audio → Voice Module → **Speaker plays speech** |

### Path Group B: BLIND SENDER (Braille Module → TEXT)

| # | Path | Full Pipeline |
|---|---|---|
| B1 | Blind → Deaf | Braille buttons → decoder → **TEXT** → Hub → Avatar sign IDs → Deaf Module → **Avatar on screen** |
| B2 | Blind → Mute | Braille → **TEXT** → Hub → TEXT → Mute Module → **Display on screen** |
| B3 | Blind → Paralyzed | Braille → **TEXT** → Hub → TEXT + TTS → Paralyzed Module → **Screen + Speaker** |
| B4 | Blind → Parkinson's | Braille → **TEXT** → Hub → TEXT + TTS → Parkinson's Module → **Screen + Speaker** |
| B5 | Blind → Hearing | Braille → **TEXT** → Hub → TTS audio → Voice Module → **Speaker** |

### Path Group C: MUTE SENDER (Glove Module → TEXT)

| # | Path | Full Pipeline |
|---|---|---|
| C1 | Mute → Deaf | Glove+Camera → Gesture ML → **TEXT** → Hub → Avatar sign IDs → Deaf Module → **Avatar** |
| C2 | Mute → Blind | Glove+Camera → **TEXT** → Hub → Braille encoding → Blind Module → **Gear pins** |
| C3 | Mute → Paralyzed | Glove+Camera → **TEXT** → Hub → TEXT + TTS → Paralyzed Module → **Screen + Speaker** |
| C4 | Mute → Parkinson's | Glove+Camera → **TEXT** → Hub → TEXT + TTS → Parkinson's Module → **Screen + Speaker** |
| C5 | Mute → Hearing | Glove+Camera → **TEXT** → Hub → TTS audio → Voice Module → **Speaker** |

### Path Group D: PARALYZED SENDER (Gaze Module → TEXT)

| # | Path | Full Pipeline |
|---|---|---|
| D1 | Paralyzed → Deaf | Gaze cursor + blink → **TEXT** → Hub → Avatar sign IDs → Deaf Module → **Avatar** |
| D2 | Paralyzed → Blind | Gaze → **TEXT** → Hub → Braille encoding → Blind Module → **Gear pins** |
| D3 | Paralyzed → Mute | Gaze → **TEXT** → Hub → TEXT → Mute Module → **Screen** |
| D4 | Paralyzed → Parkinson's | Gaze → **TEXT** → Hub → TEXT + TTS → Parkinson's Module → **Screen + Speaker** |
| D5 | Paralyzed → Hearing | Gaze → **TEXT** → Hub → TTS audio → Voice Module → **Speaker** |

### Path Group E: PARKINSON'S SENDER (Gaze Module → TEXT)

| # | Path | Full Pipeline |
|---|---|---|
| E1 | Parkinson's → Deaf | Gaze KB (tremor-filtered) → **TEXT** → Hub → Avatar IDs → Deaf Module → **Avatar** |
| E2 | Parkinson's → Blind | Gaze KB → **TEXT** → Hub → Braille encoding → Blind Module → **Gear pins** |
| E3 | Parkinson's → Mute | Gaze KB → **TEXT** → Hub → TEXT → Mute Module → **Screen** |
| E4 | Parkinson's → Paralyzed | Gaze KB → **TEXT** → Hub → TEXT + TTS → Paralyzed Module → **Screen + Speaker** |
| E5 | Parkinson's → Hearing | Gaze KB → **TEXT** → Hub → TTS audio → Voice Module → **Speaker** |

### Path Group F: HEARING SENDER (Voice Module → audio → Hub STT → TEXT)

| # | Path | Full Pipeline |
|---|---|---|
| F1 | Hearing → Deaf | Mic → audio stream (32kbps) → Hub STT → **TEXT** → Avatar IDs → Deaf Module → **Avatar** |
| F2 | Hearing → Blind | Mic → Hub STT → **TEXT** → Braille encoding → Blind Module → **Gear pins** |
| F3 | Hearing → Mute | Mic → Hub STT → **TEXT** → TEXT → Mute Module → **Screen** |
| F4 | Hearing → Paralyzed | Mic → Hub STT → **TEXT** → TEXT + TTS → Paralyzed Module → **Screen + Speaker** |
| F5 | Hearing → Parkinson's | Mic → Hub STT → **TEXT** → TEXT + TTS → Parkinson's Module → **Screen + Speaker** |

---

## 7. THE 5 NOVEL RESEARCH CONTRIBUTIONS (Updated)

### 🏆 Contribution 1: Cross-Disability Remote Communication Protocol (STRONGEST)
No existing system enables modality-agnostic remote messaging where a sender's physical input is automatically translated into a receiver's physical output over an encrypted IoT network. The **modular ecosystem approach** (specialized modules + shared hub + cloud relay) is architecturally unprecedented in assistive technology.

### 🏆 Contribution 2: 6×6 Communication Matrix (30+ Paths)
No single system handles all permutations of deaf ↔ blind ↔ mute ↔ paralyzed ↔ Parkinson's ↔ hearing communication. The inclusion of neurodegenerative conditions and the formalized routing matrix is unprecedented.

### 🏆 Contribution 3: Modular Ecosystem Architecture
The **modular design** — where each user owns only a specialized, low-cost I/O module and shares a Central Hub for processing — is a genuinely novel IoT topology for assistive technology. This enables scalability (add new disability modules without redesigning the system), affordability ($15–90 per user vs. $300+), and ecosystem growth.

### 🏆 Contribution 4: Tremor-Tolerant Gaze Keyboard
Open-source, low-cost gaze keyboard ($50–65 module vs. Tobii Dynavox $5000+) with Gaussian tremor filtering, adaptive dwell-time calibration, and direct routing to any output modality via the hub.

### 🏆 Contribution 5: Edge-Deployed Hybrid Sensor-Vision Fusion
Dual-stream Bi-LSTM fusion of glove sensor data + MediaPipe Holistic landmarks runs **locally on the module** (RPi Zero 2W) — no data leaves the module until it's converted to TEXT. This captures all 5 linguistic parameters of sign language with zero additional hardware.

---

## 8. HYBRID SENSOR-VISION FUSION (On Glove Module)

### 8.1 The Problem

Sign language has **5 linguistic parameters**. A glove alone captures only **2/5**. Changing any one parameter changes the meaning entirely (MOTHER vs. FATHER = same handshape, different location).

| # | Parameter | Glove Alone | Camera (MediaPipe) | Hybrid Fusion |
|---|---|---|---|---|
| 1 | **Handshape** | ✅ Flex sensors (primary) | ✅ Hand landmarks (backup) | ✅✅ |
| 2 | **Palm Orientation** | ✅ BNO085 IMU (primary) | ✅ Hand landmarks (backup) | ✅✅ |
| 3 | **Movement Path** | ⚠️ IMU accel only | ✅ Pose wrist tracking | ✅✅ |
| 4 | **Location** | ❌ Cannot capture | ✅ Pose landmarks (body-relative) | ✅✅ **Solved** |
| 5 | **Non-Manual Markers** | ❌ Cannot capture | ✅ Face Mesh 468 landmarks | ✅✅ **Solved** |

### 8.2 How It Works on the Glove Module

```
  WITHIN THE GLOVE MODULE (no external streaming)
  ┌──────────────────────────────────────────────────────────────────┐
  │                                                                  │
  │  ┌────────────────┐   Serial/I2C   ┌────────────────────────┐  │
  │  │  ESP32-S3      │ ─────────────► │  RPi Zero 2W           │  │
  │  │  (Glove MCU)   │               │  (Vision Processor)     │  │
  │  │                │               │                          │  │
  │  │  5× Flex vals  │               │  Camera → MediaPipe     │  │
  │  │  IMU quaternion │               │  Holistic (543 lmarks)  │  │
  │  │  Health data   │               │                          │  │
  │  │  100+ Hz       │               │  Temporal alignment      │  │
  │  └────────────────┘               │  Feature fusion (~1600)  │  │
  │                                    │  Bi-LSTM inference       │  │
  │                                    │                          │  │
  │                                    │  OUTPUT: TEXT "Hello"    │  │
  │                                    │          (95.3% conf)    │  │
  │                                    └───────────┬──────────────┘  │
  │                                                │ WiFi            │
  └────────────────────────────────────────────────┼────────────────┘
                                                   ▼
                                           Central Hub receives TEXT
```

**Everything happens locally within the module.** Only the final TEXT result leaves the module — no raw sensor data, no camera frames, no landmarks are sent over the network.

### 8.3 Performance on RPi Zero 2W

| Metric | Value | Notes |
|---|---|---|
| MediaPipe Holistic FPS | ~5–8 FPS at 320×240 | Lite model on Cortex-A53 |
| Bi-LSTM inference | ~50ms per prediction | TFLite optimized |
| Signing window | 30–60 frames | 5–12 seconds at 5–8 FPS — sufficient for individual signs |
| Estimated accuracy | ~90–96% on focused vocabulary | Depends on training data size |

> For production, RPi 4 CM (~$35) or RPi 5 CM would significantly improve FPS. The research prototype demonstrates feasibility.

---

## 9. COMMUNICATION PROTOCOL

### 9.1 Local Communication (Module ↔ Central Hub)

Uses **MQTT locally** — the Central Hub runs a Mosquitto broker that all modules connect to via WiFi.

**Module → Hub topics:**
- `ability/local/{module_id}/input` — module sends TEXT (or audio for Voice Module)
- `ability/local/{module_id}/health` — module sends health data

**Hub → Module topics:**
- `ability/local/{module_id}/output` — hub sends converted output data
- `ability/local/{module_id}/status` — hub sends system status

### 9.2 Remote Communication (Hub ↔ Cloud ↔ Hub)

Uses **MQTT over internet** with AES-256-GCM E2EE.

**Hub → Cloud topics:**
- `ability/remote/{target_hub_id}/inbox` — encrypted TEXT payload

**Cloud → Hub topics:**
- `ability/remote/{my_hub_id}/inbox` — hub subscribes to its own inbox

### 9.3 AMP (Ability Message Protocol)

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

---

## 10. TECHNOLOGY STACK SUMMARY

### Software on Each Component

| Component | Software | Language | Key Libraries |
|---|---|---|---|
| **ESP32-S3** (Glove/Braille/Voice) | Firmware | C/C++ (Arduino/ESP-IDF) | Kalman filter, BLE/WiFi, MQTT client, I2C |
| **RPi Zero 2W** (Glove Module) | Sign language recognition | Python | MediaPipe Holistic, TensorFlow Lite (Bi-LSTM), Paho MQTT |
| **RPi Zero 2W** (Gaze Module) | Eye-tracking / Gaze keyboard | Python | MediaPipe Iris, NumPy (Gaussian filter), Paho MQTT |
| **Central Hub (RPi 5)** | Hub engine | Python | Vosk/Whisper (STT), Piper (TTS), Paho MQTT, Mosquitto, PyCryptodome (AES-256-GCM), Flask/PyQt (UI) |
| **Cloud VPS** | MQTT broker | — | Mosquitto v5.0, Let's Encrypt TLS |

---

## 11. IMPLEMENTATION ROADMAP

### Phase 0: Research & Proposal ✅ COMPLETE
- [x] Literature review (40+ papers, 2024–2025)
- [x] Novelty analysis — 5 contributions identified
- [x] System architecture design (modular ecosystem)
- [x] BOM finalization (per-module + hub)
- [x] Hybrid fusion approach design
- [x] Communication matrix (6×6, 30+ paths)
- [x] AMP protocol specification
- [x] Security model (E2EE)
- [x] Course proposal slides (23 slides)
- [x] Research proposal slides (11 slides)
- [x] Target journals identified

### Phase 1: Hardware Assembly & Bring-Up (Week 1–4)
- [ ] Procure components for all 4 module types + Central Hub
- [ ] **Glove Module:** Assemble ESP32 glove + RPi Zero 2W + camera + screen
- [ ] **Braille Module:** Assemble ESP32 + buttons + 3D print gear housing + servo motors + rack-and-pinion pins + speaker
- [ ] **Gaze Module:** Assemble RPi Zero 2W + NoIR camera + IR ring + screen + speaker
- [ ] **Voice Module:** Assemble ESP32 + mic + speaker
- [ ] **Central Hub:** Set up RPi 5 + NVMe + touchscreen
- [ ] Unit-test each module individually (sensors, actuators, cameras)
- [ ] Verify WiFi connectivity between all modules and hub
- [ ] Verify rack-and-pinion Braille actuation (all 64 patterns) — test pin height, tactile quality
- [ ] Document hardware assembly with photos

### Phase 2: Software Development — Module Firmware & Input Encoders (Week 4–9)
- [ ] **Glove Module firmware:** ESP32 sensor loop + Kalman filter + serial stream to Zero
- [ ] **Glove Module ML:** MediaPipe Holistic pipeline on Zero + hybrid fusion data collection
- [ ] **Glove Module ML:** Train Bi-LSTM on hybrid dataset (target >90% accuracy)
- [ ] **Braille Module firmware:** Button interrupt handler + Perkins decoder → TEXT
- [ ] **Gaze Module (Paralyzed):** MediaPipe Iris gaze mapping + EAR blink detection → TEXT
- [ ] **Gaze Module (Parkinson's):** Dwell-click QWERTY + Gaussian tremor filter + word prediction
- [ ] **Voice Module firmware:** Audio capture + WiFi streaming to Hub
- [ ] **All modules:** MQTT client connecting to local Hub broker

### Phase 3: Central Hub Software (Week 9–12)
- [ ] **Local MQTT broker:** Set up Mosquitto on RPi 5
- [ ] **STT engine:** Vosk/Whisper for Voice Module audio → TEXT
- [ ] **Format converters:** TEXT → Braille encoding, TEXT → Avatar IDs, TEXT → TTS (Piper), TEXT → display
- [ ] **Translation router:** Receives from any module, determines target, converts, delivers
- [ ] **Admin UI:** User profiles, contact list, status dashboard on 7" touchscreen
- [ ] **Conversation history:** Local storage on NVMe SSD
- [ ] **Health monitor:** Receives health data, triggers alerts
- [ ] **Remote MQTT client:** Paho MQTT connecting to cloud broker
- [ ] **E2EE:** AES-256-GCM key exchange + encrypt/decrypt
- [ ] **Device pairing:** QR code / 6-digit code for hub-to-hub pairing

### Phase 4: Cloud & Remote Testing (Week 12–15)
- [ ] Deploy Mosquitto MQTT broker on VPS
- [ ] Test local routing: Module A → Hub → Module B (same hub)
- [ ] Test remote routing: Module A → Hub A → Cloud → Hub B → Module B
- [ ] Test ALL 30 cross-disability paths (local and remote)
- [ ] Measure end-to-end latency per path (target: <500ms)
- [ ] Measure per-stage latency: module processing → WiFi → hub → network → remote hub → module
- [ ] Test network resilience (high latency, packet loss, reconnection)
- [ ] Test offline message queuing
- [ ] Measure power consumption per module (battery life)
- [ ] Build comparison table vs. 3–5 existing systems

### Phase 5: User Study & Paper Writing (Week 15–20)
- [ ] Obtain ethics/IRB approval
- [ ] Recruit 5–10 participants across disability profiles
- [ ] Conduct System Usability Scale (SUS) evaluation
- [ ] Conduct NASA-TLX cognitive load assessment
- [ ] Measure Braille reading speed (blind participants)
- [ ] Measure avatar comprehension (deaf participants)
- [ ] Measure gaze keyboard WPM + error rate (Parkinson's participants)
- [ ] Write paper (see Section 12 for structure)
- [ ] Prepare figures, tables, experimental data
- [ ] Internal review and revision
- [ ] Submit to IEEE Internet of Things Journal

---

## 12. PAPER STRUCTURE (FOR Q1 JOURNAL)

**Proposed Title:**
> *"Ability: A Modular IoT Ecosystem for Cross-Disability Remote Communication with Multi-Modal Sensory Translation and Tremor-Tolerant Gaze Input"*

| Section | Pages | Content |
|---|---|---|
| **1. Introduction** | 1.5 | Problem (800M+ affected), gap (no cross-disability remote), 5 contributions |
| **2. Related Work** | 2.0 | Sign language systems, Braille tech, eye-tracking, multi-modal platforms, IoT assistive → show NO cross-disability remote ecosystem exists |
| **3. Ecosystem Architecture** | 3.0 | Modular design (4 module types + hub + cloud), per-module hardware, AMP protocol, security model |
| **4. Multi-Modal Translation** | 2.0 | 6 input encoders, 4 output renderers, hybrid fusion ML, TEXT intermediate, routing logic |
| **5. Implementation** | 2.0 | Module photos, software stack, network testbed, cost analysis per module |
| **6. Results & Evaluation** | 3.0 | Accuracy (confusion matrix, F1), per-path latency, per-module power, user study (SUS, NASA-TLX) |
| **7. Discussion** | 1.5 | Comparison table, module cost vs. commercial, limitations, future work |
| **8. Conclusion** | 0.5 | Summary of contributions, ecosystem impact |

---

## 13. TARGET JOURNALS (RANKED)

| Priority | Journal | IF | Q | Fit |
|---|---|---|---|---|
| 🥇 1st | **IEEE Internet of Things Journal** | 8.2 | Q1 | Modular IoT ecosystem + MQTT + E2EE |
| 🥈 2nd | **IEEE Sensors Journal** | 4.5 | Q1 | Multi-sensor fusion, wearable modules |
| 🥉 3rd | **Sensors (MDPI)** | 3.4 | Q1/Q2 | Open-access, fast review |
| 4th | **IEEE Access** | 3.4 | Q1 | Broad scope, social impact |
| 5th | **ACM TACCESS** | 3.6 | Q2 | Accessible computing (needs user study) |
| 6th | **Scientific Reports (Nature)** | 3.8 | Q1 | Interdisciplinary |

---

## 14. KEY DESIGN DECISIONS & RATIONALE

| Decision | Choice | Rationale |
|---|---|---|
| **Architecture** | Modular ecosystem (not monolithic per-user) | Each user gets only what they need; cheaper, more scalable, zero hardware waste |
| **Module processor (vision)** | RPi Zero 2W (not ESP32 alone) | ESP32 can't run MediaPipe; Zero 2W is cheapest option with camera + Linux |
| **Module processor (non-vision)** | ESP32-S3 | Sufficient for buttons, mic, servo PWM; very low power and cost |
| **Central Hub** | RPi 5 8GB (not cloud server) | Local processing for privacy; no dependency on internet for local use |
| **Hub role** | Format conversion + routing (not ML inference) | ML runs on modules to keep data local; hub only handles TEXT routing and output conversion |
| **TEXT intermediate** | All paths go through TEXT | Reduces complexity from O(n²) to O(n); 6 encoders + 4 renderers = 10, not 30 |
| **Local communication** | MQTT on local WiFi | Consistent with remote MQTT; lightweight; supports pub/sub pattern |
| **Voice Module audio** | Stream to hub (32kbps) | STT is too heavy for ESP32; 32kbps audio is negligible bandwidth (not video) |
| **Avatar output** | Pre-stored animation clips (not real-time 3D) | RPi Zero 2W can't render 3D; hub sends sign IDs, module plays clips |
| **IMU** | BNO085 (not MPU6050 for orientation) | Built-in fusion, drift-free; MPU6050 only for fall detection |
| **ML model** | Bi-LSTM (not Transformer) | Lighter for RPi Zero 2W inference; sufficient for sign classification |
| **Fusion** | Late fusion | Different sampling rates (100Hz glove vs. 30FPS camera); late fusion preserves features |
| **Encryption** | AES-256-GCM + TLS 1.3 | E2EE so cloud never sees plaintext; TLS protects transport |
| **STT/TTS** | Offline (Vosk/Piper) | Privacy; works without internet |
| **Protocol** | MQTT v5.0 | QoS 2 exactly-once; offline queuing; designed for IoT |

---

## 15. CRITICAL REQUIREMENTS FOR PAPER ACCEPTANCE

> **A concept-only paper will be desk-rejected from any Q1 journal.**

### Must-Have
- [ ] Working prototype of all 4 module types + Central Hub
- [ ] Trained Bi-LSTM with accuracy >90% on defined vocabulary
- [ ] Real end-to-end latency measurements per path
- [ ] Remote communication demo over real internet
- [ ] At least one full cross-disability path working (e.g., Sign → Remote Braille)
- [ ] User study with 5+ participants across disability profiles
- [ ] Comparison table vs. existing systems
- [ ] Per-module power consumption data
- [ ] Per-module cost analysis
- [ ] Ethics/IRB approval

### Strengthens Paper
- [ ] Multiple gesture vocabularies
- [ ] Network resilience testing
- [ ] Braille reading speed with blind users
- [ ] Avatar comprehension with deaf users
- [ ] Gaze keyboard WPM with Parkinson's users
- [ ] Cost comparison vs. Tobii ($5000+), commercial Braille displays ($3000+)
- [ ] Open-source release

---

## 16. REAL-WORLD USE CASES

1. **Family Home** — Central Hub in the living room. Deaf son uses Glove Module, blind mother uses Braille Module. Son signs "dinner is ready" → mother feels ⠙⠊⠝⠝⠑⠗ under her fingertips. Mother types back "coming" → son sees avatar signing it.

2. **Cross-Country Family** — Deaf grandson (Rahim, Dhaka) has Hub A. Blind grandmother (Fatima, Sylhet) has Hub B. Rahim signs "I miss you" → his Glove Module converts to TEXT → Hub A encrypts → MQTT cloud → Hub B decrypts → converts to Braille → Fatima's Braille Module actuates ⠠⠊ ⠍⠊⠎⠎ ⠽⠕⠥.

3. **Hospital Clinic** — Hub in the ward. Paralyzed patient uses Gaze Module to type "chest pain" → Hub routes to doctor's Voice Module → plays TTS "chest pain." Doctor speaks response → Hub STT → routes to patient's screen.

4. **Parkinson's Independence** — Mr. Karim (68) uses Gaze Module with tremor-filtering keyboard. Types "Happy birthday" → Hub encrypts → MQTT → deaf granddaughter's Hub → converts to Avatar commands → her Glove Module plays avatar signing "Happy birthday." She signs back → his module plays TTS.

---

## 17. DOCUMENT INDEX

| File | Purpose |
|---|---|
| `PROJECT_BRAIN.md` | Master reference & TODO tracker — **read this first** |
| `plan/MASTER_PLAN.md` | **THIS FILE** — comprehensive research & implementation plan |
| `.agents/rules/project-context.md` | Auto-loading AI context rule |
| `Ability_Project_Proposal.pdf` | Original project proposal |
| `anti/novelty_analysis.md` | Literature review + Q1 publishability analysis |
| `anti/project_ability_slide_deck.md` | 23-slide course proposal presentation guide |
| `anti/research_proposal_slides.md` | 11-slide academic research pitch |
| `anti/sign_language_fix.md` | Hybrid sensor-vision fusion technical analysis |

---

> **This plan is a living document. Update it as implementation progresses.**
