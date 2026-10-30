# Ability Parkinson Eye-Gaze Keyboard

This adapter reuses the tested MediaPipe face tracker and the current saved
calibration in `Electronic_Shawon/calibration.json`. It adds Parkinson-friendly
median gaze smoothing and connects the keyboard as `gaze_01` through EMQX TLS.

## Start

Install the added MQTT dependency in the Electronic_Shawon environment once:

```bash
cd Electronic_Shawon
.venv-mac/bin/python -m pip install -r requirements.txt
cd ..
./gaze.sh
```

Do not use the copied `.venv-mac/bin/activate` script after moving the project;
it contains the environment's old absolute path. The launcher above resolves
the current project path and invokes the correct Python directly.

The existing calibration loads automatically. Press `C` only when a new
calibration is required.

Use the **GAZE ENG/বাংলা** selector in Ability Chat to change the keyboard
live. Both layouts reuse the same saved calibration, smoothing, and blink
timing; switching language does not recalibrate the camera.

## Eye controls

- Look left/right to move across keys.
- Short two-eye blink moves down one keyboard row.
- Close both eyes deliberately for 0.8 seconds to select.
- Select `TO ABIR`, `TO BRAILLE`, or `TO VOICE` to choose a recipient.
- Select `SEND` to publish the typed text to Ability Chat.
- `HELP` and `WATER` publish their emergency phrase immediately.

The device publishes presence to `ability/v1/gaze_01/status`, typed messages to
`ability/v1/gaze_01/input`, listens on `ability/v1/gaze_01/output`, and receives
the retained language setting on `ability/v1/gaze_01/control`.
