# Integration Tests

Cross-module integration tests — verifies communication between modules and the Central Hub.

## Files to Create

| File | Tests |
|---|---|
| `test_module_to_hub.py` | Module publishes TEXT → Hub receives and processes |
| `test_hub_to_module.py` | Hub sends converted output → Module receives correctly |
| `test_hub_routing.py` | Module A → Hub → Module B (local routing) |
| `test_mqtt_connection.py` | MQTT connect/disconnect/reconnect resilience |
