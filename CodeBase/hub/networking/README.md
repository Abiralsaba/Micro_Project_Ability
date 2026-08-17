# Hub Networking

MQTT client (local + remote), E2EE encryption, and AMP protocol handler.

## Files to Create

| File | Purpose |
|---|---|
| `mqtt_local.py` | Local MQTT broker management + client for module communication |
| `mqtt_remote.py` | Remote MQTT client connecting to cloud relay for cross-hub messaging |
| `encryption.py` | AES-256-GCM encrypt/decrypt + key exchange utilities |
| `amp_handler.py` | AMP protocol serialization/deserialization (JSON ↔ Python objects) |
