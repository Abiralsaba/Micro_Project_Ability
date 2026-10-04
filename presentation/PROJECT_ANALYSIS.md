# Project Ability — presentation-oriented project analysis

Reviewed 30 September 2026. Scope: repository architecture, hub implementation, glove recognition, Braille input/output variants, cloud transport, voice skeleton, ML/gaze plans, setup scripts and prior research notes. This is a source review with limited mocked routing checks. It is not a hardware, clinical or model-accuracy evaluation. The older paper audit predates several current files and is not treated as the current implementation inventory.

## 1. Strongest project idea

Ability’s compelling idea is **a modular communication ecosystem**: people use different physical interfaces, but messages share a text representation and a Raspberry Pi routing hub. This is a good presentation story because it links the hardware to a human interaction instead of presenting isolated sensors.

The three core source-backed functions suitable for a rehearsed demonstration are:

1. Glove sensor input and rule-based character/gesture recognition.
2. Received text rendered by a four-cell mechanical Braille display.
3. Braille character entry using six dot buttons plus an action button.

MQTT routing is a fourth useful feature. The latest Braille cloud receiver also contains TLS connectivity, status events and a one-message pending slot. Source presence does not demonstrate physical reliability or that a particular combination is currently flashed.

## 2. What is actually implemented

| Area | Source evidence | Accurate presentation statement |
|---|---|---|
| Glove sensing | `firmware/glove_flex_imu/glove_mqtt.ino`; `firmware/glove_module/glove_module.ino` | ESP32-S3 reads five flex inputs and BNO055 orientation; calibration and gesture rules exist. |
| Glove messaging | `publishText()` and SEND/SPACE handling in `glove_mqtt.ino` | Recognized symbols build a sentence, which can be published as text. Serial text injection is also supported for transport testing. |
| Local hub | `hub/main.py` | Pi subscribes to glove, gaze/voice text and status topics; forwards text toward Braille and voice-command topics. The Braille device drives the servos. |
| Cloud hub | `hub/cloud_hub.py` | MQTT v5/TLS client accepts text and a target; converts text into Braille patterns for `braille_01`, or forwards text to other known targets. Uses QoS 1. |
| Four-cell display | `firmware/braille_module/braille_mqtt.ino`; `firmware/braille_cloud/braille_cloud.ino` | Four six-dot cells use eight servos and calibrated angle tables. Letters and spaces are supported; this is not a complete literary Braille implementation. |
| Button entry | `readKeyboard()`, `commitPendingPattern()`, `sendFullBufferToServos()` in current cloud sketch | Six dot buttons accumulate a pattern; action commits, spaces, locally displays or clears. Current alphabet decoder matches hub A–Z patterns. |
| Browser interface | Embedded HTML and HTTP handlers in Braille sketches | Text entry, dot preview and status UI exist. The preview is not actual pin-position telemetry. |
| Voice | `firmware/voice_module/voice_module.ino` | I2S capture scaffold exists; playback callback still contains TODO, and full STT/TTS integration is absent. |
| AMP | `shared/amp_schema.json` | A proposed protocol schema exists. The actual local/cloud message formats are different and are not automatically schema-validated. |

### Two real transport variants

**Local:** glove publishes `ability/glove/text` → `hub/main.py` → `ability/braille/display` carrying text → local Braille firmware.

**Cloud:** processed-text producer publishes `ability/v1/{source}/input` with a `target` → `hub/cloud_hub.py` → `ability/v1/braille_01/output` carrying dot patterns → cloud Braille firmware.

The local glove currently does not publish the cloud input format. These paths need a matching producer/receiver or an explicit bridge. The deck shows their common architectural principle without pretending that mixing the two configurations works automatically.

## 3. Claims that need qualification

**“Pi is the main processing hub” is supportable as the architecture.** It performs routing and, in the cloud variant, text-to-Braille encoding. It does not perform all computation: the glove ESP32 currently classifies gestures, and the Braille ESP32 handles buttons, timing and servo control. For the proposed custom model, the deck selects Pi 5 as a future inference host; older plans instead place vision/fusion on Pi Zero 2 W. That future placement requires implementation and benchmarking.

**“Our own trained model” is a future milestone in this checkout.** `docs/ml_pipeline.md` describes sensor/vision fusion and Bi-LSTM training, but no training implementation, dataset, model weights or evaluation output was found, including a scan for common model artifact extensions. The firmware classifier is a chain of conditions on finger bend and orientation. It lacks dynamic J/Z handling and does not establish natural sign-language translation. Do not use a confidence example or planning estimate as an accuracy result.

**“Glove + avatar” is the intended two-way interface, with an unimplemented receiver here.** No avatar asset set or renderer was found. `glove_mqtt.ino` logs received output; it does not animate a character. Translation quality would also require a defined sign language and linguistically valid animations, rather than assuming word-for-word spoken text is sign language.

**“Braille buttons send to other modules” remains an integration step for the current cloud keyboard.** `sendFullBufferToServos()` calls `displayWord()` locally. It does not publish the typed text to a hub input topic. An older generic Braille sketch has a different MQTT design; that does not complete the current cloud keyboard return path.

**“Worldwide / encrypted” needs the transport scope stated.** Cloud TLS is present. The current hub does not implement the planned AES-GCM payload encryption, AMP expiry semantics or a complete delivery protocol. TLS is not equivalent to end-to-end encryption, and QoS is not proof that a physical Braille pattern was correctly presented.

**“First ever” is not established.** The repository’s later literature work is already more cautious than the initial master plan. Orbit Chat and prior EEG/EOG wheelchairs show related accessible communication and mobility systems. The defensible novelty pitch is the particular affordable, modular integration and its eventual measured benefits. Public searching cannot prove exhaustive worldwide priority.

## 4. Practical findings that affect a live presentation

| Priority | Finding | Consequence / recommendation |
|---|---|---|
| High | Seven separate `.ino` files under `firmware/braille_module/` each define `setup()` and `loop()` | Treat these as alternative sketches, not one Arduino sketch build. Use the exact rehearsed build folder. There are also two entry-point sketches under the glove flex/IMU folder. |
| High | `hub/cloud_hub.py` uses `parents[2]` after files were flattened | In this checkout the inferred base is the parent of the project, so `.env` and relative CA paths resolve incorrectly. Confirm the actual deployment layout before running a cloud demo. |
| High | `scripts/send_cloud_message.py` imports `hub.core.cloud_hub`; service/setup instructions also reference `CodeBase/hub/core/` | Those paths do not match this checkout. A saved deployment may differ; do not copy these commands into the final script without checking. |
| High | Local glove and cloud Braille use different MQTT topics and payload shapes | Do not combine these firmware variants in the demo without a tested bridge. |
| Medium | Gesture rules operate on coarse finger states and a limited vocabulary | Demonstrate rehearsed supported gestures; do not claim full ASL or continuous sign-language accuracy. |
| Medium | Keyboard’s SEND operation is local output | Explain button entry separately from cross-module reply until its MQTT publisher and receiving UI are integrated. |
| Medium | Servo attachment and browser previews are software observations | Film the physical dots. They do not establish pin position, tactile force, reader comprehension or servo wiring correctness. |
| Medium | Cloud receiver holds one pending message and rejects a further message when full | Test pacing and rejection behavior. Do not promise unlimited queues or guaranteed end-to-end delivery. |
| Medium | Unsupported cloud text characters become blank patterns; firmware variants differ | Use known supported letters/spaces; do not claim full punctuation, numerals, contractions or Bangla Braille. |
| Medium | Browser status JSON is manually concatenated from strings | Quotes/backslashes can break it; avoid treating the UI as robust deployment evidence. |
| Medium | Voice capture/playback and topic families do not complete a working speech path | Keep speech as planned in the presentation unless a separate demonstrated implementation is supplied. |

No firmware, hub or setup files were changed to address these findings; the request was analysis and presentation material. Existing edits to the Braille cloud source and untracked keyboard/servo sketches were preserved. Private credential contents were not read or copied into the presentation.

## 5. Proposed wheelchair module — presentation only

The new module combines **mobility selection** and **communication**, sharing the Ability hub. Target users include some people with Parkinson’s or other motor limitations who need alternatives to hand controls. Suitability cannot be inferred from the diagnosis alone; Parkinson’s may affect vision and eye movements too.

| Proposed subsystem | Candidate hardware / function | What remains to be developed |
|---|---|---|
| EEG acquisition | OpenBCI Cyton 8-channel board as a research reference, plus appropriate electrodes | Signal acquisition, artifact rejection, calibrated user-specific intent classification and repeatability tests |
| Eye input | Raspberry Pi Camera Module 3 NoIR or calibrated camera setup | Gaze estimation/calibration, head-pose compensation, dwell selection and false activation tests |
| Main processing | Raspberry Pi 5 | Separate EEG and gaze pipelines; explicit drive versus communication mode; custom-model inference benchmarking |
| Gaze keyboard | Existing display, large targets and dwell-to-select | Typing, corrections, explicit send, receiver selection and MQTT message integration |
| Local motor supervision | Dedicated ESP32-S3 controller as a prototype candidate; wheelchair-rated motor interface | Independent timeout, E-stop, obstacle-stop logic, speed limits and validated braking behavior |
| Mobility base | Rated powered-wheelchair base, motors/brakes and battery | Compatibility, power budget, fail-safe interface and supervised validation; no small hobby driver assumed adequate |
| Message delivery | Text → Pi → accessible receiver | Reuse Braille routing; add text UI, avatar and speech renderers as separate integration work |

**Proposed interaction:** user explicitly chooses drive mode or communication mode. Drive mode turns confirmed EEG intent or gaze selection into bounded commands. The local controller supervises movement. Communication mode stops the chair, opens the gaze keyboard and sends composed text to the chosen Ability module. Cloud connectivity must not be required for stopping.

EEG detects electrical activity; it does not read arbitrary thoughts. Eye blink/muscle artifacts must not be presented as evidence of neural intent decoding. MediaPipe Iris landmarks alone are not a calibrated gaze tracker. Start with a non-moving or benchtop rig, then staged supervised evaluation before considering occupied use.

## 6. Cost and performance framing

The revised budget is in **Bangladeshi taka**. The owner supplied a glove estimate of **৳4,000–5,000**; the working midpoint is **৳4,500**. Component allocations under that total are cost targets, not itemized receipts. The remaining totals are **design-to-cost targets**: four-cell Braille ৳3,500, voice ৳1,500, gaze ৳3,500, and a shared Raspberry Pi hub ৳15,000. Small supplier variations of roughly ৳50–100 do not establish these as actual purchase prices.

Core glove + Braille + hub = **৳23,000**. With the proposed voice/gaze interfaces, communication-system parts target = **৳28,000**, or **৳30,800 with a 10% reserve**. The lower hub figure assumes a lower-memory or reused Pi 5, an existing screen and economy sourcing. It is not a quote for a new Pi 5 8GB kit. Confirm model memory/inference requirements before selecting a lower-memory board. The gaze budget changes the older Pi Zero + NoIR arrangement to an economy USB camera sharing the Pi hub and existing screen; this is a proposed cost-reduction design, not an existing implementation.

The **৳12,000 wheelchair add-on target** covers only a proposed control/safety interface allocation. EEG acquisition, electrodes, the powered chair, motors, brakes, traction battery and validation remain separate and unpriced. It is not the total cost of a brain-controlled wheelchair. Labor, production testing, tax, shipping, model training and software development are not included in the communication parts totals. No already-purchased expensive part was silently assigned an invented lower actual price.

Selected premium comparison: Rokoko Smartgloves II official listing **USD 1,995** → **৳245,385**, about **৳2.45 lakh**. HumanWare Brailliant BI 40X live listing **USD 3,759** → **৳462,357**, about **৳4.62 lakh**. Both use the disclosed rounded conversion **USD 1 ≈ ৳123**, grounded in Bangladesh Bank’s 30 September 2026 interbank WAR of 123.03. The HumanWare search snippet showed an older promotional price; the opened product and category pages both showed 3,759, which was used. Professional glove pair vs single gesture glove and 40-cell commercial reader vs four-cell prototype are labeled differences; no equivalent-performance or direct savings percentage is claimed. Orbit Reader 20, about **৳98,300**, remains visible as a lower-price commercial reference. See SOURCES.md for links and exact conversions.

Sharing the ৳15,000 target hub among five users allocates **৳3,000 per user**. That share must be added when estimating a deployed personal interface. Sharing capacity, real supplier costs and total ownership cost remain to be established.

The revised AI/chat slide is an illustrative platform design, with example messages and planned contacts/history/quick requests. It is not a screenshot of a running hub application. Current source evidence is unchanged.

For performance, collect participant/session-separated gesture results, false activations, message latency, all 64 tactile pattern checks per cell, and user comprehension. Report trial counts and failure conditions. The static 26-letter lookup agreement in SOURCE_CHECKS.json is **not** recognition accuracy, mechanical accuracy or reading accuracy.

## 7. Checks performed

- Parsed and loaded both hub Python files with a mocked MQTT dependency; no broker connection.
- Checked a local mock `HELP` message publishes to the intended Braille and voice-command topics.
- Checked the cloud mock route generates `[19, 17, 7, 15]` for `HELP` and the expected Braille output topic.
- Compared the current cloud keyboard’s 26 alphabet entries with the cloud hub map: all match. This supersedes the older audit’s mapping observation for this specific newer variant only.
- Confirmed multiple Arduino entry points in the alternative sketch folder and the cloud base-path mismatch.
- Scanned for common trained model artifacts; none found.
- Rendered and inspected the presentation, checked text bounds and navigation, and structurally checked PowerPoint slides/transitions. See VALIDATION.json.

No ESP32 compilation, flashing, tactile load measurement, real MQTT delivery trial, avatar playback, model training, EEG acquisition, wheelchair control or participant study was performed.
