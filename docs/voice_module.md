# Voice Module — Hearing/Sighted Users

> **Hardware:** ESP32-S3 + Mic + Speaker
> **Cost:** ~$15–25

## What This Module Does

The Voice Module serves **Hearing/Sighted** users — the bridge between the disabled and non-disabled communities. The user speaks into a microphone, audio is streamed to the Central Hub (32 kbps — lighter than a phone call), and the Hub runs STT to convert speech → TEXT. When receiving, the Hub sends TTS audio → module plays on speaker.

## Data Flow

```
INPUT:  Mic (INMP441) → ESP32 → Audio Stream (32 kbps) → WiFi → Central Hub → STT → TEXT
OUTPUT: Central Hub → TTS Audio → WiFi → ESP32 → I2S Amp → Speaker
```

## Why Stream Audio Instead of Local STT?

STT engines (Vosk/Whisper) need more compute than ESP32 can provide. Audio streaming at 16kHz mono = ~32 kbps over WiFi — this is negligible bandwidth (less than a phone call). The Central Hub's RPi 5 handles STT efficiently.

## Hardware

| Component | Purpose |
|---|---|
| ESP32-S3 WROOM | Module MCU |
| INMP441 I2S MEMS Mic | Voice capture |
| MAX98357A I2S Amp + 3W Speaker | Audio output |
| Optional: Small OLED (1.3") | Status display |
