# Glove Module — Deaf & Mute Users

> **Hardware:** ESP32-S3 (glove sensors) + RPi Zero 2W (camera + ML)
> **Cost:** ~$70–90

## What This Module Does

The Glove Module serves **Deaf** and **Mute** users. It captures sign language / gestures using a sensor glove and camera, converts them to **TEXT** locally using hybrid sensor-vision fusion, and sends the TEXT to the Central Hub via WiFi.

## Subdirectories

| Directory | Language | Runs On | Purpose |
|---|---|---|---|
| `firmware/` | C/C++ (Arduino/ESP-IDF) | ESP32-S3 | Reads flex sensors, IMU, health sensors. Streams data to RPi Zero via serial |
| `vision/` | Python | RPi Zero 2W | Runs MediaPipe Holistic + Bi-LSTM hybrid fusion. Outputs TEXT to Hub via MQTT |

## Data Flow

```
Flex Sensors ──►┐
IMU (BNO085) ──►┤ ESP32-S3 ──serial──► RPi Zero 2W ──WiFi──► Central Hub
Health Sensors ─┘                      + Camera
                                       + MediaPipe Holistic
                                       + Bi-LSTM Model
                                       = TEXT output
```

## Hardware Connections (ESP32-S3)

| Pin | Component | Protocol |
|---|---|---|
| GPIO 34-39 (ADC) | ADS1115 → Flex Sensors | I²C (SDA/SCL) |
| GPIO 21/22 | BNO085 IMU | I²C |
| GPIO 16/17 | MAX30102 (SpO2) | I²C |
| GPIO 18/19 | MPU6050 (Fall Detection) | I²C |
| TX/RX | RPi Zero 2W | Serial UART |

## Software Modes

- **Deaf Mode:** Sign language recognition → TEXT. Receives avatar sign IDs from Hub → plays clips on screen.
- **Mute Mode:** Gesture recognition → TEXT. Receives plain text from Hub → displays on screen.
