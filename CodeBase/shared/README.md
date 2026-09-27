# Shared Libraries

Reusable code shared across multiple components of the ecosystem.

## Modules

### `protocol/` — AMP (Ability Message Protocol)
JSON schema definitions, serialization/deserialization utilities for the AMP protocol used between modules, hubs, and cloud.

### `crypto/` — End-to-End Encryption
AES-256-GCM encryption/decryption utilities, key exchange helpers. Used by the Central Hub for remote message encryption.

### `braille/` — Braille Encoding/Decoding
Braille Grade 1 lookup tables, text-to-braille encoder, braille-to-text decoder. Used by both the Braille Module firmware and the Hub's Braille converter.
