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
Flex Sensors (ADS1115) ──►┐
BNO055 IMU (orientation) ─►┤ ESP32-S3 ──serial──► RPi Zero 2W ──WiFi──► Central Hub
BMP280 (temp/pressure) ───►┤                      + Camera
MAX30102 (SpO2/HR) ───────►┤                      + MediaPipe Holistic
MPU6050 (fall detection) ──┘                      + Bi-LSTM Model
                                                  = TEXT output
```

## Hardware Connections (ESP32-S3)

| Pin | Component | Protocol | I²C Address |
|---|---|---|---|
| GPIO 21/22 (SDA/SCL) | ADS1115 → 5× Flex Sensors | I²C | 0x48 |
| GPIO 21/22 (SDA/SCL) | BNO055 IMU (palm orientation) | I²C | 0x28 |
| GPIO 21/22 (SDA/SCL) | BMP280 (barometric/temperature) | I²C | 0x76 or 0x77 |
| GPIO 21/22 (SDA/SCL) | MAX30102 (SpO2 + heart rate) | I²C | 0x57 |
| GPIO 21/22 (SDA/SCL) | MPU6050 (fall detection) | I²C | 0x68 |
| GPIO 17 (TX) / 16 (RX) | RPi Zero 2W | Serial UART | — |
| GPIO 4 | Battery voltage divider | ADC | — |

## Required Arduino Libraries

Install via Arduino IDE Library Manager:
- `Adafruit ADS1X15` — 16-bit ADC for flex sensors
- `Adafruit BNO055` — 9-DOF IMU with on-chip fusion
- `Adafruit Unified Sensor` — dependency for BNO055 & BMP280
- `Adafruit BMP280` — barometric pressure + temperature
- `SparkFun MAX3010x` — SpO2 + heart rate
- `Adafruit MPU6050` — fall detection accelerometer
- `ArduinoJson` — JSON serialization for UART/MQTT payloads
- `PubSubClient` — MQTT client

## Serial UART Data Format (ESP32 → RPi Zero)

Each line is a JSON object at 100 Hz:
```json
{
  "ts": 12345,
  "f": [16234, 15890, 14200, 13500, 2048],
  "q": [0.98, 0.01, -0.05, 0.12],
  "e": [180.5, 12.3, -5.7],
  "la": [0.15, -0.08, 0.32],
  "cal": 3
}
```

| Key | Description |
|---|---|
| `ts` | Timestamp (millis) |
| `f[0-4]` | Flex sensor ADC values (thumb, index, middle, ring, pinky) |
| `q[0-3]` | Quaternion (w, x, y, z) — primary orientation for ML |
| `e[0-2]` | Euler angles (heading, roll, pitch) — human-readable |
| `la[0-2]` | Linear acceleration (gravity removed) — gesture velocity |
| `cal` | BNO055 system calibration status (0=uncalibrated, 3=fully calibrated) |

## Software Modes

- **Deaf Mode:** Sign language recognition → TEXT. Receives avatar sign IDs from Hub → plays clips on screen.
- **Mute Mode:** Gesture recognition → TEXT. Receives plain text from Hub → displays on screen.

## MQTT Topics

| Topic | Direction | Data |
|---|---|---|
| `ability/local/glove_user_01/health` | ESP32 → Hub | Heart rate, SpO2, fall status, battery, IMU calibration |
| `ability/local/glove_user_01/environment` | ESP32 → Hub | Temperature, pressure, altitude (from BMP280) |
| `ability/local/glove_user_01/output` | Hub → ESP32 | Avatar commands, display text (forwarded to RPi) |
