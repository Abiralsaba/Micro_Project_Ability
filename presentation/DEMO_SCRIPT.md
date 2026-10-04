# Live hardware demonstration — approximately 2:45

Slide 7 is a handoff only. Stop screen sharing or press B and switch the recording to the hardware camera. Keep slides off throughout the demonstration. The deck resumes on slide 8.

## 0:00–0:20 — show the real system

Wide shot: glove, Raspberry Pi, Braille board and its power supply. Name the exact boards fitted. Trace sensor wiring, hub connection and the regulated servo supply; explain the common ground. Mount and label the boards; secure cables so the tactile mechanism stays visible. Use a close-up camera angle for the pins and buttons.

Say: “The glove senses finger bend and hand orientation. Raspberry Pi is our shared processing and routing hub. The Braille controller converts the message into physical dots.”

## 0:20–1:20 — feature 1: glove message through the Pi

Use the local route already represented by `firmware/glove_flex_imu/glove_mqtt.ino`, `hub/main.py` and `firmware/braille_module/braille_mqtt.ino`, **if these match the flashed devices**. Rehearse the actual deployed combination. The cloud Braille sketch uses a different topic/payload family and is not a drop-in receiver for this local glove route.

1. Show open and bent hand calibration and live flex/BNO055 values.
2. Enter a short message using gestures that you have verified. `HELP` is a four-character candidate, not a claim of measured recognition success. If any gesture is unreliable, choose a rehearsed supported alternative.
3. Show the recognized characters in the serial output, then the SEND gesture.
4. Show the Pi’s received message and routing log, then pan to the tactile output.

Explain input → sensors → ESP32 recognition → MQTT message → Raspberry Pi route. Keep any manual text injection clearly labeled **transport test**, never “gesture recognition.”

## 1:20–1:55 — feature 2: tactile output

Close-up of four Braille cells. Explain that each cell has six dots and two MG90S servos. For `HELP`, the six-bit patterns are `19, 17, 7, 15` in decimal (hex `13, 11, 07, 0F`). Show the raised dots, hold period and reset.

Explain: “Servo-driven cams select the dot patterns. These are four real tactile cells.” Browser previews and `Servo.attached()` status do not prove physical pin correctness. Do not claim reading accuracy from the visual demonstration.

## 1:55–2:30 — feature 3: Braille button input

Use the already-flashed keyboard-capable board/sketch. Do not reflash during the video. If the integrated local route lacks keyboard support, show this as a **separate module demonstration**, with a clear camera cut and accurate explanation.

For the current cloud keyboard sketch: press dot 1, then action once to commit `A`. Press dots 1 and 2, then action once to commit `B`. Triple-click action within the configured click window to show the buffer on the local Braille cells. Double-click adds a space; hold action to clear. Demonstrate only the operations actually confirmed on your hardware.

Say: “The six dot buttons enter a character. The action button commits it, inserts a space or displays the buffer.” The present function `sendFullBufferToServos()` displays locally. It does **not** publish a reply through the hub. Describe avatar reception as the next integration step.

## 2:30–2:45 — demonstrate stability and return

Show one fresh message after reset, then the assembled system. Say: “These three functions form our current prototype foundation. Next, here is how we keep the system affordable.” Resume slide 8.

Do not insert wheelchair animation into this live demonstration. The proposed wheelchair remains on slide 10, clearly labeled as a concept.

## Rubric coverage

| Category | Weight | Evidence / location |
|---|---:|---|
| Idea & novelty | 15% | Slides 1–3; modular integration and shared text, bounded novelty |
| Real-life impact | 15% | Slide 6; classroom, home and proposed care scenarios |
| Features & hardware | 40% | Slides 3–5; three live input/output demonstrations above |
| Cost effectiveness | 10% | Slides 8–9 and component BOM appendix |
| Tidiness & professionalism | 10% | Labeled hardware, secured wires, clear pin close-up, consistent slide design |
| Presentation | 10% | Emotional opening, concise narration, visible demonstration, clear closing |

The implementation earns hardware credit through observed behavior, not animation. If a route is unavailable on the filming day, label the narrower demonstration accurately rather than narrating an unshown capability.
