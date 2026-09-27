# Glove Module Vision — MediaPipe + ML Fusion (Python on RPi Zero 2W)

Runs on the Glove Module's RPi Zero 2W. Handles camera capture, MediaPipe Holistic landmark extraction, hybrid fusion with glove sensor data, and Bi-LSTM inference.

## Files to Create

| File | Purpose |
|---|---|
| `main.py` | Entry point — starts camera, serial listener, MQTT client |
| `mediapipe_pipeline.py` | MediaPipe Holistic — extracts 543 landmarks (33 pose + 42 hand + 468 face) |
| `serial_listener.py` | Reads sensor data from ESP32 via Serial UART (flex + IMU at 100Hz) |
| `fusion.py` | Temporal alignment + feature concatenation (glove + vision → ~1600 features) |
| `inference.py` | Loads TFLite Bi-LSTM model, runs prediction on sliding window → TEXT |
| `mqtt_client.py` | Sends TEXT to Central Hub, receives avatar commands for display |
| `avatar_player.py` | Plays pre-stored sign animation clips on LCD screen |
