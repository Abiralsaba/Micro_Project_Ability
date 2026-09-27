# Hub Health Monitoring

Receives health data from Glove Modules (SpO2, heart rate, fall detection) and triggers alerts.

## Files to Create

| File | Purpose |
|---|---|
| `health_monitor.py` | Listens to health MQTT topics, applies thresholds, triggers alerts |
| `alerts.py` | Alert dispatch — notification on Hub screen, optional remote alert to contacts |
