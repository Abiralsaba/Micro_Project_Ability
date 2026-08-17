# Shared — E2E Encryption (AES-256-GCM)

Encryption utilities for secure remote communication between Central Hubs.

## Files to Create

| File | Purpose |
|---|---|
| `aes_gcm.py` | AES-256-GCM encrypt/decrypt functions using PyCryptodome |
| `key_exchange.py` | ECDH key exchange for establishing shared secrets between hubs |
| `key_store.py` | Local key storage and retrieval (per-contact encryption keys) |
