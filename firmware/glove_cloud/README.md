# Secure cloud glove firmware

`glove_cloud.ino` is the EMQX/TLS version of
`../glove_flex_imu/glove_flex_imu.ino`.

The flex pins, BNO055 pins, ten-second calibration, 400 ms gesture
confirmation, and classifier are unchanged. In particular, the open-hand
orientation still classifies as `5`; cloud commands do not replace signs.

The reference glove has no physical send button, so a sentence is published
after the hand returns to neutral for 2.5 seconds. Enter `send` in Serial
Monitor to publish immediately, or `clear` to discard the sentence.

## Cloud topics

- Input: `ability/v1/glove_01/input`
- Output: `ability/v1/glove_01/output`
- Presence: `ability/v1/glove_01/status`

The sketch reuses `../braille_cloud/secrets.h` and `emqx_ca.h`, ensuring both
modules connect to the same broker with the same TLS trust configuration.

Compile with:

```bash
arduino-cli compile --fqbn esp32:esp32:esp32s3 firmware/glove_cloud
```
