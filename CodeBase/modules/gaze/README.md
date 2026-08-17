# Gaze Module — Paralyzed & Parkinson's Users

> **Hardware:** RPi Zero 2W + NoIR Camera + IR LEDs
> **Cost:** ~$50–65

## What This Module Does

The Gaze Module serves **Paralyzed** (ALS, spinal cord injury) and **Parkinson's** users. It uses eye-tracking via MediaPipe Iris to let users type text using only their eyes, then sends the TEXT to the Central Hub.

## Software Modes

### Paralyzed Mode
- Camera → MediaPipe Iris → gaze coordinates → cursor on screen
- Eye Aspect Ratio (EAR) detects intentional blinks = "click"
- User navigates on-screen keyboard by looking + blinking

### Parkinson's Mode
- Same camera pipeline + **Gaussian smoothing** on gaze coordinates
- Filters out involuntary tremors and micro-saccades
- **Dwell-click:** user stares at a key for 400–1500ms (adaptive) to "press" it
- **Word prediction** reduces keystrokes by ~50%

## Data Flow

```
Camera → RPi Zero 2W → MediaPipe Iris → Gaze Coordinates
  → [Paralyzed: Cursor + Blink] OR [Parkinson's: Tremor Filter + Dwell]
  → TEXT → WiFi → Central Hub

Central Hub → Display Text → Screen
Central Hub → TTS Audio → Speaker
```

## Hardware

| Component | Purpose |
|---|---|
| RPi Camera Module 3 NoIR (Wide) | Eye capture in all lighting |
| 850nm IR LED Ring | Invisible illumination for reliable tracking |
| Small LCD (3.5–5") | Gaze keyboard UI / text display |
| Speaker + MAX98357A | TTS audio output |
