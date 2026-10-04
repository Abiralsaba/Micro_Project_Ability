# Ability Parkinson Eye-Gaze Keyboard

This adapter reuses the tested MediaPipe face tracker and the current saved
calibration in `Electronic_Shawon/calibration.json`. It adds Parkinson-friendly
median gaze smoothing, connects the keyboard as `gaze_01` through EMQX TLS,
and controls the complete Ability Chat interface when `Parkinson User` is the
active sender.

## Start

Install the added MQTT dependency in the Electronic_Shawon environment once:

```bash
cd Electronic_Shawon
.venv-mac/bin/python -m pip install -r requirements.txt
cd ..
./gaze_module/start_gaze.sh
```

Do not use the copied `.venv-mac/bin/activate` script after moving the project;
it contains the environment's old absolute path. The launcher above resolves
the current project path and invokes the correct Python directly.

The existing calibration loads automatically. Press `C` only when a new
calibration is required.

## Eye controls

- In Ability Chat, switch the active sender to `Parkinson User`.
- Look around the display to move the turquoise eye cursor.
- Hold the cursor over any button for 1.2 seconds, or look at it and blink, to
  activate it. This works for contacts, toolbar controls, quick messages, Send,
  and the identity selector.
- Look at `EYE KEYBOARD` in the header to show the on-screen keyboard. Looking
  at the message box also opens it. Use `HIDE KEYBOARD` to return to the full UI.
- In the camera preview, look left/right to move across its compact keys.
- Short two-eye blink moves down one keyboard row.
- Close both eyes deliberately for 0.8 seconds to select.
- Select `TO ABIR`, `TO BRAILLE`, or `TO VOICE` to choose a recipient.
- Select `SEND` to publish the typed text to Ability Chat.
- `HELP` and `WATER` publish their emergency phrase immediately.

The device publishes presence to `ability/v1/gaze_01/status`, app cursor frames
to `ability/v1/gaze_01/cursor`, typed messages to
`ability/v1/gaze_01/input`, and listens on `ability/v1/gaze_01/output`.

Horizontal cursor movement uses the saved left/right calibration. Vertical
movement is measured from each iris relative to its eyelids, so the same saved
calibration remains valid and there is no second calibration file.
