# Braille Cloud Firmware

Upload `braille_cloud.ino` to the ESP32 that controls the four Braille cells.

It connects directly to EMQX with TLS, subscribes to
`ability/v1/braille_01/output`, queues up to four messages, and displays the
six-dot patterns produced by the Raspberry Pi. It does not translate text.

Install these Arduino components:

- ESP32 board package by Espressif Systems
- PubSubClient
- ArduinoJson 7.x
- ESP32Servo

Keep `braille_cloud.ino`, `emqx_ca.h`, and `secrets.h` in the same sketch
folder. `secrets.h` is ignored by Git.
