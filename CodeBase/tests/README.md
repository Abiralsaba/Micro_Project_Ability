# Tests

## Structure

| Directory | Purpose |
|---|---|
| `unit/` | Per-module unit tests (sensor reads, Braille decode, gaze logic) |
| `integration/` | Cross-module tests (Module → Hub → Module routing) |
| `e2e/` | End-to-end path tests (all 30 cross-disability paths) |

## Running Tests

```bash
# All tests
pytest tests/

# Unit tests only
pytest tests/unit/

# Integration tests
pytest tests/integration/

# Specific module tests
pytest tests/unit/test_braille_decoder.py
```
