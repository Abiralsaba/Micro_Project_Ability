# Project Ability — speaker notes

12 presentation slides + 3 optional appendices. Approximately 5 minutes of narration plus 2 minutes 45 seconds of live hardware.

## 01. Everyone deserves a way to say hello.

[20 seconds] Imagine having something important to say, but the person beside you cannot receive it in the way you express it. A simple hello should not depend on sight, hearing, or steady hands. We are [team names], and this is Project Ability: different ways to communicate, one connection. Replace the editable team line before recording.

## 02. Three interfaces. One shared conversation.

[25 seconds] Our idea connects personal interfaces through one shared hub. The glove turns selected hand gestures into messages. The Braille unit lets a user enter characters and feel received text. Our proposed third module adds a gaze keyboard and brain or eye-based wheelchair commands. The common language inside the system is text. The avatar, full return path and mobility extension still need integration; this is the ecosystem direction, not a claim that every route is deployed.

## 03. Our own AI/ML processing hub.

[30 seconds] Our platform vision is an AI/ML-based processing hub built on Raspberry Pi 5, using our own task-specific recognition model and an accessible chat interface. It brings model inference, message routing, receiver selection and format conversion into one place. The present repository implements routing and Braille encoding; custom model inference, chat history, avatar and speech remain integration milestones. Today the glove classifier runs on its ESP32. This diagram presents the intended architecture, not a claim that all these features are deployed.

## 04. A hand movement becomes a message.

[25 seconds] Five flex sensors measure finger bend, and a BNO055 measures orientation. The ESP32-S3 calibrates the values, applies gesture rules and builds text. A send gesture publishes the message to the Raspberry Pi. This is a limited gesture and fingerspelling prototype, not full sign-language translation. Our next recognition step is a custom-trained sensor-and-vision model. A signing avatar is also part of the planned receive interface, but its renderer and clips are absent from this checkout.

## 05. Read with touch. Reply with buttons.

[30 seconds] The latest Braille firmware has four six-dot cells with eight MG90S servos: two servos per cell. Cam mechanisms convert servo position into dot patterns. Six dot buttons accumulate a character, and the action button commits, spaces or displays the buffer. Show the actual button sequence during the demo. In the current cloud firmware, triple-click sends the buffer to its own display, not back through MQTT. Sending that reply to the glove or avatar remains an integration step. The letters shown here are the correct six-dot patterns for HELP; physical correctness still needs verification.

## 06. Small moments. Real independence.

[20 seconds] We are targeting practical moments: a classroom partner sharing a short message, a family communicating across different access needs, and eventually a person with motor limitations composing a request with their eyes. These are intended use scenarios. The benefits we want to measure are successful conversations, fewer access barriers and easier interaction. People have different abilities and preferences, so the appropriate interface must be selected with each user.

## 07. Now, follow one message.

[10 seconds, then 2 minutes 45 seconds LIVE HARDWARE, NO SLIDES] Say: Now let us follow one message through the real hardware. End screen sharing or press B to black out the browser. Follow DEMO_SCRIPT.md: show boards/power, glove input, hub receipt and four tactile cells; then show Braille button entry and local output. Use a text injection only as an explicitly labeled transport test. Do not represent keyboard-to-avatar or wheelchair animation as working hardware. After the demonstration return directly to slide 08. This is a handoff, not a slide substituting for the demo.

## 08. Built for a local budget.

[30 seconds] All amounts are in Bangladeshi taka. You supplied a glove estimate of 4,000 to 5,000 taka; we use 4,500 as the working midpoint. The other amounts are design-to-cost targets, not receipts: Braille 3,500, voice 1,500, gaze 3,500 and shared Pi hub 15,000. The core glove, Braille and hub target is 23,000. Adding voice and gaze brings the communication-system target to 28,000, or 30,800 with a ten-percent reserve. The lower hub target assumes a lower-memory or reused Pi 5 and an existing screen; model performance needs benchmarking. The optional wheelchair controller target of 12,000 excludes EEG acquisition and the powered chair. It is not the cost of a full brain-controlled wheelchair. No tax, shipping, labor or clinical validation cost is included.

## 09. Premium prices. A lower-cost direction.

[30 seconds] The price barrier is substantial. At the rounded presentation conversion of 123 taka per US dollar, Rokoko Smartgloves II cost about 2.45 lakh taka for a professional motion-capture pair. Our single gesture-glove estimate is 4,000 to 5,000 taka. HumanWare Brailliant BI 40X is about 4.62 lakh taka for a commercial 40-cell reader, while our four-cell prototype target is 3,500 taka. These are selected premium references, not the entire market or equivalent-performance comparisons. The product pages opened today show 1,995 and 3,759 US dollars respectively; an older search snippet showed a HumanWare sale price that the current page did not. Lower-cost commercial options also exist: Orbit Reader 20 is about 98,300 taka. Ability costs exclude a personal allocation of the shared hub, production support, tax and shipping. The affordability strategy is focused functions, shared processing and locally assembled parts.

## 10. The next module: mobility + a voice.

[40 seconds] This is our proposed brain-and-eye wheelchair module, shown only as a concept. It targets people who need an alternative to hand controls, including some people with Parkinson’s after individual assessment. A multichannel EEG interface would classify a small set of trained intentions; a camera-based gaze interface offers another way to choose directions. In communication mode, the chair is stopped and a gaze keyboard sends text through the same Raspberry Pi to Braille, text, and eventually avatar or speech. A separate local controller must enforce an emergency stop, obstacle stop, command timeout and speed limit. It should never depend on cloud connectivity for stopping. Gaze calibration and real EEG classification remain research work. This is neither a medical claim nor demonstrated wheelchair hardware.

## 11. One hub. One accessible chat.

[30 seconds] This is the chat-interface design for our own AI/ML-based hub. The intended workflow combines recognition with receiver selection and accessible delivery. A glove or gaze user composes a message; the hub sends it as Braille, on-screen text or, after integration, avatar or speech. We plan contacts, conversation history, quick requests and device status in one interface. The bubbles shown here are an illustrative conversation, not live device output. Our next engineering milestones are training the custom model, integrating the chat interface and measuring accuracy, false activations and end-to-end delay on unseen users.

## 12. Different abilities. Shared possibilities.

[15 seconds] Project Ability brings sensing, embedded control and message routing into one modular design. The current code gives us gesture input, a tactile display, button entry and a hub-routing foundation. Our next challenge is to validate the experience and complete the return paths. The goal is simple: different abilities, shared possibilities. Thank you.

## 13. Core build: ৳23,000 target.

Backup slide. All amounts are Bangladeshi taka cost targets. The glove total of 4,500 follows the owner’s 4,000–5,000 estimate; its line items are allocations, not actual receipts. Braille totals 3,500; Pi hub 15,000. The proposed lower hub target uses a lower-memory or reused Pi 5, not a newly purchased 8GB kit. Economy sourcing and model-inference performance must be checked. Core total is 23,000. Voice adds 1,500 and gaze adds 3,500, yielding 28,000, or 30,800 with 10 percent reserve. Optional wheelchair interface adds 12,000 before EEG, electrodes, powered chair/motors/brakes/battery and testing. The full wheelchair cost is not established. All component rows, including optional modules, are in budget.csv.

## 14. What the evidence supports

Backup for judge questions. Static source inspection is not physical validation. See PROJECT_ANALYSIS.md for file-specific findings and limitations. Avoid first-ever, perfect accuracy, full sign-language translation, end-to-end encrypted or all routes implemented claims. Orbit Chat and prior EEG/EOG wheelchair papers establish overlap with broad first-ever statements. The compelling contribution is this particular affordable modular integration, to be validated.

## 15. Sources & presentation scope

Public sources checked September 30, 2026. Click the source titles in the browser, PDF or PowerPoint. The deck is based on static review of this checkout and the user’s requested vision; no hardware was tested and no clinical or accuracy outcomes are claimed. Longer source notes and price assumptions are in SOURCES.md.