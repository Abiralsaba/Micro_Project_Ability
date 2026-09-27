# Hub STT (Speech-to-Text)

Offline speech recognition engine. Converts audio from the Voice Module into TEXT.

## Engine Options

| Engine | Model Size | Accuracy | Speed | Notes |
|---|---|---|---|---|
| **Vosk** | ~50 MB | Good | Fast | Recommended for RPi 5 |
| **Whisper** (tiny/base) | 75-150 MB | Better | Slower | Higher accuracy, more compute |

## Files to Create

| File | Purpose |
|---|---|
| `stt_engine.py` | Wrapper around Vosk/Whisper — receives audio chunks, returns TEXT |
| `models/` | Directory for downloaded STT model files |
