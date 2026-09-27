# Gaze Module Source — Eye-Tracking (Python on RPi Zero 2W)

Runs on the Gaze Module's RPi Zero 2W. Handles camera capture, MediaPipe Iris eye-tracking, and text input via gaze.

## Files to Create

| File | Purpose |
|---|---|
| `main.py` | Entry point — detects user profile (paralyzed/parkinsons), launches correct mode |
| `eye_tracker.py` | MediaPipe Iris pipeline — camera → gaze coordinates + blink detection (EAR) |
| `gaze_cursor.py` | Paralyzed mode — maps gaze to screen cursor, blink = click |
| `gaze_keyboard.py` | Parkinson's mode — QWERTY layout, Gaussian tremor filter, dwell-click, word prediction |
| `mqtt_client.py` | Connects to Central Hub MQTT, sends TEXT, receives display/TTS data |
| `display.py` | Renders keyboard UI / text display on the small LCD screen |
