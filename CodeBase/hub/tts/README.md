# Hub TTS (Text-to-Speech)

Offline speech synthesis engine. Converts TEXT into audio for modules with speakers.

## Engine: Piper TTS

- Fully offline, runs on RPi 5
- Multiple voice options per language
- Low latency (~200ms for short sentences)

## Files to Create

| File | Purpose |
|---|---|
| `tts_engine.py` | Wrapper around Piper — receives TEXT, returns audio bytes |
| `voices/` | Directory for downloaded voice model files |
