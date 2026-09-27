# Hub Format Converters

Converts TEXT into the target module's output format.

## Files to Create

| File | Purpose |
|---|---|
| `text_to_braille.py` | TEXT → Braille byte array (Grade 1 encoding) for Blind modules |
| `text_to_avatar.py` | TEXT → Sign sequence IDs (e.g., `["HELLO","HOW","YOU"]`) for Deaf modules |
| `text_to_tts.py` | TEXT → TTS audio bytes via Piper for modules with speakers |
| `text_to_display.py` | TEXT → formatted display string for modules with screens |
