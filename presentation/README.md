# Project Ability — presentation package

Open **Ability_Presentation.html** in a browser for the animated version. It is self-contained and works offline. Use **Ability_Presentation.pptx** for editable PowerPoint text, diagrams, speaker notes and fade transitions. **Ability_Presentation.pdf** is the static sharing/printing version. **Preview_Contact_Sheet.jpg** previews all 15 slides.

The main talk has **12 slides**, followed by **3 optional appendices**. Allow roughly **7–8 minutes including a 2 minute 45 second hardware demonstration**. Animation is strongest in the browser; PowerPoint has slide fades, and PDF is static.

## Present

- Arrow keys / Space: advance; Home: opening; End: closing (slide 12).
- F: fullscreen; N: speaker notes; B: black screen for the hardware handoff; Esc: close overlays.
- A: 18-second auto advance for rehearsal. It stops at the hardware handoff and closing. Manual timing is recommended for the real talk.
- The bottom controls appear on hover or keyboard focus. Select an appendix from the slide menu. Touch swipes also navigate.
- After slide 7, show **actual hardware with no slides**. Resume at slide 8 after the demo. Follow DEMO_SCRIPT.md.

## Before recording

Replace the team placeholders on slide 1. Confirm which Pi and ESP32 variants are physically installed. All slide prices are now in **BDT**. The glove uses your ৳4,000–5,000 estimate (৳4,500 midpoint). Other components are lower-cost design targets; replace allocations with actual receipts before calling the total expenditure. Small local-price variations of about ৳50–100 are expected. Add measured performance only when its test conditions and trial count are known.

The deck deliberately distinguishes source-backed functions from planned interfaces. The wheelchair is a **presentation-only proposal**, and no firmware was added for it. The current checkout has no trained model weights, accuracy report or avatar renderer. The architecture now prominently presents **our own AI/ML processing hub** and a **chat-interface concept** with contacts, history, quick requests and accessible output. Brief concept/integration labels preserve the distinction from the present repository. No model accuracy is invented.

## Supporting files

- PROJECT_ANALYSIS.md: current implementation, gaps and findings that affect the presentation.
- DEMO_SCRIPT.md: filming order, exact input/output explanations and rubric coverage.
- SPEAKER_NOTES.md: narration and timing; also embedded in PowerPoint.
- SOURCES.md: official price references, related work and technical qualifications.
- budget.csv: component-level BDT targets; core ৳23,000, communication system with proposed voice/gaze ৳28,000, or ৳30,800 with reserve. Optional wheelchair interface ৳12,000 excludes EEG and the chair.
- VALIDATION.json: browser/layout/PPTX checks. SOURCE_CHECKS.json: limited static source checks.

## Rebuild

Use a separate Python environment with `python-pptx`, `playwright` and `Pillow` installed. Install Playwright Chromium with `python -m playwright install chromium`. Then:

```sh
python presentation/build_deck.py
python presentation/validate_deck.py
```

Edit the scene text and team line in build_deck.py to regenerate all formats consistently. Editing only the PPTX will not update the HTML/PDF. PDF export uses the browser version, not a PowerPoint rendering. The PPTX was structurally validated; inspect it in your presentation application before the event because font layout can differ.

For the final video, use the Google Drive folder supplied at registration, set access to **Anyone with the link → Viewer**, verify the link while signed out, and submit by the committee’s announced deadline. No video upload or sharing settings were changed by this task.
