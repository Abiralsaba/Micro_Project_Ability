# Braille Module — Project Ability

> **Hardware:** ESP32 + 2× MG90S Servo Motors + Rack-and-Pinion Gear Mechanism  
> **Cost:** ~$20–30  
> **Firmware:** `brailv1.ino` (Serial Monitor test) / `braille_module.ino` (Full WiFi/MQTT)

---

## Table of Contents

- [What This Module Does](#what-this-module-does)
- [Components You Need](#components-you-need)
- [Wiring Guide — Step by Step](#wiring-guide--step-by-step)
  - [ESP32 Pinout Reference](#esp32-pinout-reference)
  - [Wiring the MG90S Servos](#1-wiring-the-mg90s-servos-most-important)
  - [Power Supply](#2-power-supply)
  - [Complete Wiring Table](#3-complete-wiring-table)
  - [Wiring Diagram (Text)](#4-wiring-diagram-text)
- [How the Mechanism Works](#how-the-mechanism-works)
- [Firmware Setup — brailv1.ino](#firmware-setup--brailv1ino)
- [How to Use](#how-to-use)
- [Calibration Guide](#calibration-guide)
- [Braille Encoding Reference](#braille-encoding-reference)
- [Troubleshooting](#troubleshooting)

---

## What This Module Does

The Braille Module serves **Blind** users. It uses a **2-servo rack-and-pinion gear mechanism** with 6 metallic pins to physically display Braille characters.

**brailv1.ino** (test firmware):
1. You type a word in the **Serial Monitor** (e.g., "hello")
2. The ESP32 converts each letter to its Braille pattern
3. Two MG90S servos move to the correct positions
4. The cam-profiled racks raise the correct pins
5. Each letter is held for 2 seconds, then servos return to 0°

---

## Components You Need

| # | Component | Quantity | Purpose | Approx. Cost |
|---|-----------|----------|---------|---------------|
| 1 | **ESP32 Dev Board** (any variant — ESP32, ESP32-S3, etc.) | 1 | Main microcontroller | $5–8 |
| 2 | **MG90S Micro Servo** (Metal Gear) | 2 | Left servo (dots 1,2,3) + Right servo (dots 4,5,6) | $3–4 each |
| 3 | **Breadboard** (full size or half) | 1 | For prototyping connections | $2–3 |
| 4 | **Jumper Wires** (Male-to-Male, Male-to-Female) | ~10 | Connections | $1–2 |
| 5 | **USB Cable** (Micro-USB or USB-C, depending on your ESP32) | 1 | Programming + power for testing | — |
| 6 | **5V Power Supply** or **5V 2A USB adapter** | 1 | Power for servos (see power section) | $2–3 |
| 7 | **Capacitor 470µF 16V** (electrolytic) | 1 | Smooth servo power (optional but recommended) | $0.20 |

> **Optional for full build:** 3D printed rack bars, pinion gears, pin housing, 6× metallic pin rods (2mm), 6× buttons (Perkins layout), speaker + MAX98357A amp.

---

## Wiring Guide — Step by Step

### ESP32 Pinout Reference

Your ESP32 dev board has pins labeled on both sides. The key pins we use:

| ESP32 Pin | What It Is |
|-----------|------------|
| `GPIO 13` | Digital output — Left servo signal |
| `GPIO 12` | Digital output — Right servo signal |
| `GND`     | Ground (0V reference) |
| `5V` / `VIN` | 5V output (when powered via USB) |
| `3V3`     | 3.3V output (DO NOT use for servos) |

---

### 1. Wiring the MG90S Servos (Most Important!)

Each MG90S servo has **3 wires** coming from its connector:

```
MG90S Servo Wire Colors:
┌─────────────┬──────────────────────┐
│ Wire Color  │ Purpose              │
├─────────────┼──────────────────────┤
│ BROWN       │ Ground (GND)         │
│ RED         │ Power (+5V)          │
│ ORANGE      │ Signal (PWM control) │
└─────────────┴──────────────────────┘

Note: Some servos use Black/Red/Yellow or Black/Red/White.
      Black = GND, Red = Power, Yellow/White = Signal
```

#### LEFT SERVO (Controls Dots 1, 2, 3)

| Servo Wire | Connect To | Notes |
|------------|-----------|-------|
| **BROWN** (GND) | ESP32 **GND** pin | Any GND pin on ESP32 works |
| **RED** (+5V) | **5V power supply +** | NOT ESP32 3.3V! See power section |
| **ORANGE** (Signal) | ESP32 **GPIO 13** | This is the PWM signal wire |

#### RIGHT SERVO (Controls Dots 4, 5, 6)

| Servo Wire | Connect To | Notes |
|------------|-----------|-------|
| **BROWN** (GND) | ESP32 **GND** pin | Same GND rail as left servo |
| **RED** (+5V) | **5V power supply +** | Same 5V rail as left servo |
| **ORANGE** (Signal) | ESP32 **GPIO 12** | This is the PWM signal wire |

---

### 2. Power Supply

> ⚠️ **IMPORTANT:** MG90S servos draw up to **700mA each under load**. The ESP32's onboard voltage regulator CANNOT supply enough current for 2 servos reliably.

#### Option A: USB Power (for basic testing, low load)

If you're just testing with **no physical load** on the servos (no rack/pinion attached), you can power everything from USB:

```
USB Cable → ESP32 USB Port
ESP32 5V/VIN pin → Servo RED wires (both)
ESP32 GND pin → Servo BROWN wires (both)
```

> ⚠️ This may cause the ESP32 to **brown out and reset** if both servos move at once under load. Use Option B for reliable operation.

#### Option B: External 5V Supply (Recommended for actual use)

Use a separate 5V 2A (or higher) power supply for the servos:

```
5V Power Supply (+) → Servo RED wires (both)
5V Power Supply (-) → Servo BROWN wires (both)
5V Power Supply (-) → ESP32 GND pin  ← CRITICAL! Common ground!

ESP32 powered separately via USB cable.
```

> ⚠️ **CRITICAL:** The power supply GND and ESP32 GND **MUST be connected together** (common ground). Without this, the servo signal won't work!

#### Optional: Add a Decoupling Capacitor

Place a **470µF electrolytic capacitor** across the 5V and GND power rails (near the servos). This absorbs current spikes when servos start moving.

```
Capacitor (+) → 5V rail
Capacitor (-) → GND rail

⚠️ Electrolytic capacitors are POLARIZED!
   The longer leg = positive (+)
   The stripe side = negative (-)
   Connecting backwards can damage it!
```

---

### 3. Complete Wiring Table

```
┌────────────────────────────────────────────────────────────────┐
│                    COMPLETE WIRING TABLE                       │
├─────────────────────┬─────────────────────┬───────────────────┤
│ FROM                │ TO                  │ WIRE COLOR / NOTE │
├─────────────────────┼─────────────────────┼───────────────────┤
│ Left Servo ORANGE   │ ESP32 GPIO 13       │ Signal (PWM)      │
│ Left Servo RED      │ 5V Power Rail (+)   │ Power             │
│ Left Servo BROWN    │ GND Power Rail (-)  │ Ground            │
├─────────────────────┼─────────────────────┼───────────────────┤
│ Right Servo ORANGE  │ ESP32 GPIO 12       │ Signal (PWM)      │
│ Right Servo RED     │ 5V Power Rail (+)   │ Power             │
│ Right Servo BROWN   │ GND Power Rail (-)  │ Ground            │
├─────────────────────┼─────────────────────┼───────────────────┤
│ ESP32 GND           │ GND Power Rail (-)  │ COMMON GROUND!    │
│ 5V Supply (+)       │ 5V Power Rail (+)   │ Servo power       │
│ 5V Supply (-)       │ GND Power Rail (-)  │ Servo ground      │
├─────────────────────┼─────────────────────┼───────────────────┤
│ Capacitor (+)       │ 5V Power Rail (+)   │ 470µF (optional)  │
│ Capacitor (-)       │ GND Power Rail (-)  │ Stripe side       │
├─────────────────────┼─────────────────────┼───────────────────┤
│ USB Cable           │ ESP32 USB port      │ Programming+Power │
└─────────────────────┴─────────────────────┴───────────────────┘
```

---

### 4. Wiring Diagram (Text)

```
                        ┌──────────────────┐
                        │     ESP32        │
                        │                  │
   USB ─────────────────┤ USB              │
                        │                  │
                        │  GPIO 13 ◄───────┼─── Left Servo ORANGE (Signal)
                        │                  │
                        │  GPIO 12 ◄───────┼─── Right Servo ORANGE (Signal)
                        │                  │
                        │  GND ────────────┼──┐
                        │                  │  │
                        └──────────────────┘  │
                                              │
                                              │  (Common Ground)
    ┌─────────────────────────────────────────┘
    │
    │   ┌───────────────────────────────────────┐
    │   │          BREADBOARD POWER RAILS       │
    │   │                                       │
    ├───┤  GND Rail (─) ◄── 5V Supply (─)      │
    │   │                                       │
    │   │  5V  Rail (+) ◄── 5V Supply (+)      │
    │   │       │                    │          │
    │   │       │  [470µF Cap]       │          │
    │   │       │    (+)─────(-)     │          │
    │   │       │              │     │          │
    │   │       │              └─────┤          │
    │   │       │                    │          │
    │   └───────┼────────────────────┼──────────┘
    │           │                    │
    │     ┌─────┴─────┐        ┌────┴──────┐
    │     │ LEFT MG90S │        │RIGHT MG90S│
    │     │            │        │           │
    └─────┤ BROWN(GND) │   ┌────┤ BROWN(GND)│
          │ RED  (+5V)◄┤   │    │ RED  (+5V)│◄── 5V Rail
          │ ORANGE(Sig)│   │    │ ORANGE(Sig)│── GPIO 12
          │  └→GPIO 13 │   │    │           │
          └────────────┘   │    └───────────┘
                           │
                     GND Rail (─)
```

---

## How the Mechanism Works

### 2-Servo Rack and Pinion

```
  Braille Cell:
  [Dot 1] [Dot 4]     LEFT COLUMN    RIGHT COLUMN
  [Dot 2] [Dot 5]     (dots 1,2,3)   (dots 4,5,6)
  [Dot 3] [Dot 6]     Left Servo     Right Servo

  Left Servo ──► Left Pinion ──► Left Rack (cam profile, 8 positions)
                                  → selectively raises Dots 1, 2, 3

  Right Servo ──► Right Pinion ──► Right Rack (cam profile, 8 positions)
                                    → selectively raises Dots 4, 5, 6

  8 × 8 = 64 patterns = ALL Braille characters ✓
```

Each rack bar has a **stepped cam surface** (3D printed) with 8 distinct height profiles. The servo rotates the pinion → rack slides to a position → cam steps push specific pins UP.

### Cam Position Table

| Position | Servo Angle | Binary | Dots Raised |
|----------|------------|--------|-------------|
| 0 | 0° | `000` | None |
| 1 | 22° | `001` | Dot 1 only |
| 2 | 44° | `010` | Dot 2 only |
| 3 | 66° | `011` | Dots 1, 2 |
| 4 | 88° | `100` | Dot 3 only |
| 5 | 110° | `101` | Dots 1, 3 |
| 6 | 132° | `110` | Dots 2, 3 |
| 7 | 154° | `111` | All three |

---

## Firmware Setup — brailv1.ino

### Prerequisites

1. **Install Arduino IDE** (v2.x recommended)  
   → [https://www.arduino.cc/en/software](https://www.arduino.cc/en/software)

2. **Add ESP32 Board Support**  
   - Go to `File → Preferences`
   - In "Additional Boards Manager URLs", add:
     ```
     https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
     ```
   - Go to `Tools → Board → Boards Manager`
   - Search "esp32" and install **"esp32 by Espressif Systems"**

3. **Install the ESP32Servo Library**  
   - Go to `Sketch → Include Library → Manage Libraries`
   - Search **"ESP32Servo"** and install it

### Upload Steps

1. Connect your ESP32 to your computer via USB
2. Open `brailv1.ino` in Arduino IDE
3. Select your board:
   - `Tools → Board → ESP32 Arduino → "ESP32 Dev Module"` (or your specific board)
   - For ESP32-S3: `Tools → Board → ESP32 Arduino → "ESP32S3 Dev Module"`
4. Select port: `Tools → Port → (your COM port or /dev/ttyUSBx)`
5. For ESP32-S3, also set: `Tools → USB CDC On Boot → "Enabled"`
6. Click **Upload** (→ button)
7. Open Serial Monitor: `Tools → Serial Monitor` or `Ctrl+Shift+M`
8. Set baud rate to **115200**
9. Set line ending to **"Newline"** (dropdown at bottom of Serial Monitor)

---

## How to Use

Once uploaded and Serial Monitor is open at **115200 baud**:

### Type a word

```
Type: hello
```

The firmware will:
1. Convert each letter to its Braille pattern
2. Move both servos to display each letter
3. Hold for 2 seconds
4. Return to 0° (home) before the next letter
5. Print detailed info for each letter

### Serial Monitor output example

```
► Received: "hello"

[Braille] Displaying 5 characters:

  [1] 'h' → 0b010011  Dots: 1,2,5  Cell: ●○ ●○ ○●
         → Left servo: 66°  |  Right servo: 44°
  [2] 'e' → 0b010001  Dots: 1,5    Cell: ●○ ○○ ○●
         → Left servo: 22°  |  Right servo: 44°
  [3] 'l' → 0b000111  Dots: 1,2,3  Cell: ●○ ●○ ●○
         → Left servo: 154° |  Right servo: 0°
  [4] 'l' → 0b000111  Dots: 1,2,3  Cell: ●○ ●○ ●○
         → Left servo: 154° |  Right servo: 0°
  [5] 'o' → 0b010101  Dots: 1,3,5  Cell: ●○ ○○ ●●
         → Left servo: 110° |  Right servo: 44°

[Braille] ✓ Word complete!
```

### Special Commands

| Command | What It Does |
|---------|-------------|
| `test`  | Runs through all 26 letters (a-z), displaying each one |
| `home`  | Resets both servos to 0° (home position) |
| `sweep` | Sweeps both servos 0°→180°→0° to verify wiring and range |

---

## Calibration Guide

After assembling the physical rack-and-pinion mechanism:

1. **Type `sweep`** in Serial Monitor to verify both servos move through their full range
2. **Type `test`** to run through all 26 letters
3. If the pins don't raise correctly, adjust the `CAM_ANGLES[]` array in the code:

```cpp
const int CAM_ANGLES[8] = {
    0,     // Position 0 — no dots (start here)
   22,     // Position 1 — adjust if dot 1 doesn't raise
   44,     // Position 2 — adjust if dot 2 doesn't raise
   66,     // Position 3 — adjust for dots 1,2
   88,     // Position 4 — adjust for dot 3
  110,     // Position 5 — adjust for dots 1,3
  132,     // Position 6 — adjust for dots 2,3
  154      // Position 7 — adjust for all three
};
```

4. Tweak each angle by ±5° until the correct pins raise for each position.

---

## Braille Encoding Reference

Standard 6-dot Braille cell:

```
[1] [4]     bit0  bit3
[2] [5]     bit1  bit4
[3] [6]     bit2  bit5
```

### Letter → Braille Dot Mapping

| Letter | Dots | Binary | Hex |
|--------|------|--------|-----|
| a | 1 | `000001` | `0x01` |
| b | 1,2 | `000011` | `0x03` |
| c | 1,4 | `001001` | `0x09` |
| d | 1,4,5 | `011001` | `0x19` |
| e | 1,5 | `010001` | `0x11` |
| f | 1,2,4 | `001011` | `0x0B` |
| g | 1,2,4,5 | `011011` | `0x1B` |
| h | 1,2,5 | `010011` | `0x13` |
| i | 2,4 | `001010` | `0x0A` |
| j | 2,4,5 | `011010` | `0x1A` |
| k | 1,3 | `000101` | `0x05` |
| l | 1,2,3 | `000111` | `0x07` |
| m | 1,3,4 | `001101` | `0x0D` |
| n | 1,3,4,5 | `011101` | `0x1D` |
| o | 1,3,5 | `010101` | `0x15` |
| p | 1,2,3,4 | `001111` | `0x0F` |
| q | 1,2,3,4,5 | `011111` | `0x1F` |
| r | 1,2,3,5 | `010111` | `0x17` |
| s | 2,3,4 | `001110` | `0x0E` |
| t | 2,3,4,5 | `011110` | `0x1E` |
| u | 1,3,6 | `100101` | `0x25` |
| v | 1,2,3,6 | `100111` | `0x27` |
| w | 2,4,5,6 | `111010` | `0x3A` |
| x | 1,3,4,6 | `101101` | `0x2D` |
| y | 1,3,4,5,6 | `111101` | `0x3D` |
| z | 1,3,5,6 | `110101` | `0x35` |

---

## Troubleshooting

### Servos don't move at all

- ✅ Check signal wires are on **GPIO 13** (left) and **GPIO 12** (right)
- ✅ Check servo power — RED wire must be on **5V**, not 3.3V
- ✅ Check GND — BROWN wire must be on same GND as ESP32
- ✅ Make sure Serial Monitor shows "Both servos at 0° — home position set"

### Servos jitter or twitch randomly

- ✅ Add a **470µF capacitor** across 5V and GND near the servos
- ✅ Use an **external 5V 2A supply** instead of USB power
- ✅ Ensure **common ground** between power supply and ESP32

### ESP32 keeps resetting / rebooting

- ✅ This is a **brownout** — the servos are drawing too much current from USB
- ✅ Use an **external 5V power supply** for the servos (Option B in wiring guide)
- ✅ You can also try: `Tools → Upload Speed → 115200` if upload fails

### Serial Monitor shows nothing

- ✅ Set baud rate to **115200**
- ✅ For ESP32-S3: Set `Tools → USB CDC On Boot → "Enabled"`
- ✅ Press the **RST/EN** button on the ESP32 to restart
- ✅ Make sure you selected the correct **COM port**

### Only one servo moves

- ✅ Swap the servo connectors to test if it's a servo hardware issue
- ✅ Type `sweep` to test each servo individually
- ✅ Check the signal wire connections (GPIO 13 vs GPIO 12)

### Characters look wrong on the physical pins

- ✅ You need to **calibrate** the `CAM_ANGLES[]` array
- ✅ Type `test` and observe which positions are off
- ✅ Adjust angles by ±5° increments in the code

---

## Data Flow

```
INPUT (brailv1.ino — test mode):
  Serial Monitor → Type word → ESP32 → Braille Pattern
  → Split into Left (dots 1,2,3) + Right (dots 4,5,6)
  → Left Servo (GPIO 13) → Left Rack → raise pins
  → Right Servo (GPIO 12) → Right Rack → raise pins

FULL SYSTEM (braille_module.ino):
  INPUT:  6× Buttons → ESP32 Braille Decoder → TEXT → WiFi → Central Hub
  OUTPUT: Central Hub → Braille Bytes → ESP32 → 2× Servo PWM
          → Left Servo → Left Rack → Dots 1,2,3 (8 positions)
          → Right Servo → Right Rack → Dots 4,5,6 (8 positions)
          → 64 total Braille patterns ✓
          Central Hub → TTS Audio → ESP32 → I2S Amp → Speaker
```
