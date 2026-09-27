# Unit Tests

Per-module unit tests. Run with `pytest tests/unit/`.

## Files to Create

| File | Tests |
|---|---|
| `test_braille_encoder.py` | Braille ↔ ASCII encoding/decoding for all characters |
| `test_amp_protocol.py` | AMP message creation, validation, serialization |
| `test_encryption.py` | AES-256-GCM encrypt/decrypt round-trip |
| `test_format_converters.py` | Text → Braille, Text → Avatar IDs, Text → TTS |
| `test_health_monitor.py` | SpO2 threshold alerts, fall detection |
