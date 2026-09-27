# Shared — Braille Encoding/Decoding

Braille Grade 1 lookup tables and encoder/decoder utilities. Used by both the Braille Module firmware (C++) and the Central Hub converter (Python).

## Files to Create

| File | Purpose |
|---|---|
| `braille_tables.py` | ASCII ↔ Braille Grade 1 lookup tables (Python, for Hub) |
| `braille_tables.h` | Same tables in C header format (for ESP32 firmware) |
| `encoder.py` | `text_to_braille("Hello")` → `[0x13, 0x05, 0x0C, 0x0C, 0x0F]` |
| `decoder.py` | `braille_to_text([0x13, 0x05, 0x0C, 0x0C, 0x0F])` → `"Hello"` |
