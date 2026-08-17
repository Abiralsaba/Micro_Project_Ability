# CodeBase — Project Ability

> **Modular Cross-Disability Communication Ecosystem**

This directory contains all source code for the Project Ability ecosystem.

---

## Directory Structure

```
CodeBase/
│
├── modules/                    # Tier 1 — Individual Disability Modules
│   ├── glove/                  # Glove Module (Deaf & Mute users)
│   │   ├── firmware/           # ESP32-S3 sensor firmware (C/C++)
│   │   └── vision/             # RPi Zero 2W — MediaPipe + ML (Python)
│   │
│   ├── braille/                # Braille Module (Blind users)
│   │   └── firmware/           # ESP32-S3 button input + servo/gear output firmware (C/C++)
│   │
│   ├── gaze/                   # Gaze Module (Paralyzed & Parkinson's users)
│   │   └── src/                # RPi Zero 2W — eye-tracking (Python)
│   │
│   └── voice/                  # Voice Module (Hearing/Sighted users)
│       └── firmware/           # ESP32-S3 mic/speaker firmware (C/C++)
│
├── hub/                        # Tier 2 — Central Hub (RPi 5)
│   ├── core/                   # Hub engine — message router, module manager
│   ├── converters/             # Format converters (TEXT → Braille/Avatar/TTS/Text)
│   ├── stt/                    # Speech-to-Text engine (Vosk/Whisper)
│   ├── tts/                    # Text-to-Speech engine (Piper)
│   ├── networking/             # MQTT client, E2EE, AMP protocol handler
│   ├── ui/                     # Admin touchscreen UI (7" display)
│   ├── health/                 # Health monitoring & alert system
│   └── config/                 # Hub configuration files
│
├── cloud/                      # Tier 3 — Cloud Relay
│   └── mqtt/                   # Mosquitto broker setup & config
│
├── ml/                         # Machine Learning Pipeline
│   ├── datasets/               # Training & evaluation data
│   │   ├── raw/                # Raw recorded data (gitignored — large)
│   │   └── processed/          # Preprocessed feature files
│   ├── models/                 # Model architectures & trained weights
│   │   ├── architectures/      # Model definition code
│   │   └── trained/            # Exported model files (gitignored — large)
│   ├── training/               # Training scripts & configs
│   └── evaluation/             # Evaluation & benchmarking scripts
│
├── shared/                     # Shared Libraries (used across components)
│   ├── protocol/               # AMP protocol (JSON schema, serialization)
│   ├── crypto/                 # E2EE utilities (AES-256-GCM)
│   └── braille/                # Braille encoding/decoding tables
│
├── tests/                      # Test Suites
│   ├── unit/                   # Per-module unit tests
│   ├── integration/            # Cross-module integration tests
│   └── e2e/                    # End-to-end path tests (all 30 paths)
│
├── docs/                       # Documentation
│   ├── hardware/               # Wiring diagrams, assembly photos, pinouts
│   ├── api/                    # Internal API docs
│   └── paper/                  # Research paper drafts & figures
│
├── scripts/                    # Utility & Automation Scripts
│   ├── setup/                  # Environment setup scripts
│   ├── deploy/                 # Deployment scripts (hub, cloud)
│   └── tools/                  # Development helper tools
│
└── assets/                     # Static Assets
    ├── avatar/                 # Sign language avatar animation clips
    │   └── clips/              # Pre-rendered sign clips (gitignored — large)
    └── sounds/                 # TTS fallback sounds, notification tones
```

## Quick Start

```bash
# 1. Clone the repo
git clone <repo-url>
cd Micro_Project_Ability/CodeBase

# 2. Set up Python environment (for Hub & modules with RPi Zero)
python3 -m venv venv
source venv/bin/activate
pip install -r requirements.txt

# 3. Set up ESP32 firmware (for Glove/Braille/Voice modules)
# Open modules/<module>/firmware/ in PlatformIO or Arduino IDE

# 4. Configure the Central Hub
cp hub/config/config.example.yaml hub/config/config.yaml
# Edit config.yaml with your settings

# 5. Start the Central Hub
python hub/core/main.py
```

## Component Mapping

| Ecosystem Component | Code Location | Language |
|---|---|---|
| Glove Module — ESP32 firmware | `modules/glove/firmware/` | C/C++ |
| Glove Module — Vision ML | `modules/glove/vision/` | Python |
| Braille Module firmware | `modules/braille/firmware/` | C/C++ |
| Gaze Module software | `modules/gaze/src/` | Python |
| Voice Module firmware | `modules/voice/firmware/` | C/C++ |
| Central Hub engine | `hub/core/` | Python |
| Format converters | `hub/converters/` | Python |
| MQTT & E2EE networking | `hub/networking/` | Python |
| ML training pipeline | `ml/training/` | Python |
| Cloud MQTT broker config | `cloud/mqtt/` | Config |
