# End-to-End Path Tests

Tests all 30 cross-disability communication paths — verifies the complete pipeline from sender module input to receiver module output.

## Files to Create

| File | Tests |
|---|---|
| `test_deaf_paths.py` | Deaf → Blind, Deaf → Mute, Deaf → Paralyzed, etc. (5 paths) |
| `test_blind_paths.py` | Blind → Deaf, Blind → Mute, etc. (5 paths) |
| `test_mute_paths.py` | All 5 Mute sender paths |
| `test_paralyzed_paths.py` | All 5 Paralyzed sender paths |
| `test_parkinsons_paths.py` | All 5 Parkinson's sender paths |
| `test_hearing_paths.py` | All 5 Hearing sender paths |
| `test_remote_paths.py` | Selected paths tested over real MQTT cloud relay |
| `test_latency.py` | Measures per-path end-to-end latency breakdown |
