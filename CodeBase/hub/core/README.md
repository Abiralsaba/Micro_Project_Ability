# Hub Core

Main hub engine — the entry point and message router for the Central Hub.

## Files to Create

| File | Purpose |
|---|---|
| `main.py` | Hub entry point — starts MQTT broker, loads config, registers modules |
| `router.py` | Message router — receives TEXT from modules, determines target, dispatches to converters |
| `module_registry.py` | Tracks connected modules (ID, type, profile, online status) |
