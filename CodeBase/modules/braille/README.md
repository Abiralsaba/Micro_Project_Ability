# Braille Module — Blind Users

> **Hardware:** ESP32-S3 + 2 Servo Motors + Rack-and-Pinion Gear Mechanism
> **Cost:** ~$20–30

## What This Module Does

The Braille Module serves **Blind** users. It has 6 tactile buttons (Perkins layout) for input and a **2-servo rack-and-pinion gear mechanism** with 6 metallic pins for output. Users type messages in Braille → module decodes to TEXT → sends to Central Hub via WiFi. When receiving, the Hub sends Braille byte encoding → ESP32 splits into left/right columns → commands 2 servos → cam-profiled racks selectively raise the correct pins.

## Data Flow

```
INPUT:  6× Buttons → ESP32 Braille Decoder → TEXT → WiFi → Central Hub
OUTPUT: Central Hub → Braille Bytes → ESP32 → 2× Servo PWM
        → Left Servo → Left Rack → Dots 1,2,3 (8 positions)
        → Right Servo → Right Rack → Dots 4,5,6 (8 positions)
        → 64 total Braille patterns ✓
        Central Hub → TTS Audio → ESP32 → I2S Amp → Speaker
```

## 2-Servo Rack and Pinion Mechanism

```
  LEFT COLUMN        RIGHT COLUMN
  [Dot 1] [Dot 4]
  [Dot 2] [Dot 5]
  [Dot 3] [Dot 6]

  Left Servo ──► Left Pinion ──► Left Rack (cam profile, 8 positions)
                                  → selectively raises Dots 1, 2, 3

  Right Servo ──► Right Pinion ──► Right Rack (cam profile, 8 positions)
                                    → selectively raises Dots 4, 5, 6

  8 × 8 = 64 patterns = ALL Braille characters ✓
```

Each rack bar has a **stepped cam surface** (3D printed) with 8 distinct height profiles. The servo rotates the pinion → rack slides to a position → cam steps push specific pins UP.

```
  Position 0 (0°):    ▽▽▽  → No dots       (000)
  Position 1 (22°):   ▲▽▽  → Dot 1 only    (001)
  Position 2 (44°):   ▽▲▽  → Dot 2 only    (010)
  Position 3 (66°):   ▲▲▽  → Dots 1,2      (011)
  Position 4 (88°):   ▽▽▲  → Dot 3 only    (100)
  Position 5 (110°):  ▲▽▲  → Dots 1,3      (101)
  Position 6 (132°):  ▽▲▲  → Dots 2,3      (110)
  Position 7 (154°):  ▲▲▲  → All three     (111)
```

## Hardware Connections (ESP32-S3)

| Pin | Component | Protocol |
|---|---|---|
| GPIO 12-17 | 6× Tactile Buttons (Perkins layout) | Digital INPUT_PULLUP |
| GPIO 18 | 1× Spacebar Button | Digital INPUT_PULLUP |
| GPIO 25 | Left Servo Motor (SG90) | PWM (50 Hz) — controls left rack (dots 1,2,3) |
| GPIO 26 | Right Servo Motor (SG90) | PWM (50 Hz) — controls right rack (dots 4,5,6) |
| GPIO 32/33 | MAX98357A I2S Amp | I2S |

## Components

| Component | Purpose |
|---|---|
| ESP32-S3 WROOM | Module MCU |
| 6× Tactile Buttons + 1 Spacebar | Braille input (Perkins layout) |
| 2× Micro Servo Motors (SG90/MG90S) | Left servo → left rack; Right servo → right rack |
| 2× Pinion Gears (3D printed) | Converts servo rotation → linear rack motion |
| 2× Rack Bars with stepped cam profile (3D printed) | 8 positions per rack → selectively raises 3 pins |
| 6× Metallic Pin Rods (2mm diameter) | Braille dots — raised/lowered by rack cam steps |
| 3D Printed Pin Housing + Frame | Holds pins in 2×3 layout + structural mount |
| Speaker + MAX98357A I2S Amp | TTS audio output |
| 3.7V 1500mAh LiPo + TP4056 | Power + charging |

**No PCA9685 needed** — only 2 servos, driven directly by ESP32 GPIO PWM.

## Braille Encoding

Standard 6-dot Braille cell (dots 1-6):
```
[1] [4]
[2] [5]
[3] [6]
```

Each character = 1 byte (6 bits used):
- Bits 0-2 → left column (dots 1,2,3) → left servo position
- Bits 3-5 → right column (dots 4,5,6) → right servo position

Example: "H" = dots 1,2,5 = `0b010011` → left = `011` (position 3) + right = `010` (position 2)
