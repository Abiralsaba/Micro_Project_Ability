/*
 * ══════════════════════════════════════════════════════════════
 * GLOVE-TO-LANGUAGE MODULE — Project Ability
 * Target: ESP32-S3 WROOM (Arduino IDE)
 * Board:  "ESP32S3 Dev Module"
 * ══════════════════════════════════════════════════════════════
 *
 * PURPOSE:
 *   Reads 5 flex sensors + BNO055 IMU (SEN0253) and recognises
 *   ASL-style hand signs.  The recognised letter / word is
 *   printed to the Serial Monitor at 115200 baud.
 *
 * WIRING (matches your physical layout):
 *   Left Side                  Right Side
 *   ─────────                  ──────────
 *   GPIO 1 → Thumb  Flex       3V3  → Flex dividers + SEN0253
 *   GPIO 2 → Index  Flex       GND  → Common ground
 *   GPIO 3 → Middle Flex       GPIO 8 → SEN0253 SDA
 *   GPIO 4 → Ring   Flex       GPIO 9 → SEN0253 SCL
 *   GPIO 5 → Pinky  Flex
 *
 * FLEX-SENSOR CIRCUIT (each finger):
 *   3V3 ──┤Flex├──┬── GPIOx (ADC input)
 *                 │
 *                10 kΩ
 *                 │
 *                GND
 *
 * LIBRARIES REQUIRED (Arduino IDE Library Manager):
 *   - Adafruit BNO055
 *   - Adafruit Unified Sensor
 *
 * BOARD SETUP (Arduino IDE):
 *   1. Board → "ESP32S3 Dev Module"
 *   2. USB CDC On Boot → "Enabled"
 *   3. Upload Speed → 921600
 */

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <utility/imumaths.h>

// ── Pin Definitions ─────────────────────────────────────────
// Flex sensor analog inputs (voltage-divider output)
#define FLEX_THUMB   1
#define FLEX_INDEX   2
#define FLEX_MIDDLE  3
#define FLEX_RING    4
#define FLEX_PINKY   5

// I²C for SEN0253 (BNO055 breakout by DFRobot)
#define IMU_SDA  8
#define IMU_SCL  9

// ── Sensor Object ───────────────────────────────────────────
Adafruit_BNO055 bno = Adafruit_BNO055(55, 0x28);
bool bnoReady = false;

// ── ESP32-S3 ADC Configuration ──────────────────────────────
// Using analogReadMilliVolts() for factory-calibrated readings.
// This gives millivolts (0–3100 mV) which is more reliable
// than raw ADC counts on ESP32-S3.

// ── Flex Sensor Calibration ─────────────────────────────────
// These are filled automatically by the 10-second calibration
// at startup.  You can also hardcode them after calibrating once.
//   STRAIGHT = millivolts when finger is flat
//   BENT     = millivolts when finger is fully curled

int FLEX_STRAIGHT[5] = { 0, 0, 0, 0, 0 };  // auto-filled at boot
int FLEX_BENT[5]     = { 0, 0, 0, 0, 0 };  // auto-filled at boot
bool calibrated = false;

// ── Finger Bend Percentages (0 = straight, 100 = fully bent)
int bend[5] = { 0, 0, 0, 0, 0 };

// ── Raw millivolt values (for calibration / debugging) ──────
int rawMV[5] = { 0, 0, 0, 0, 0 };

// ── IMU Data ────────────────────────────────────────────────
float pitch = 0, roll = 0, yaw = 0;   // Euler angles (degrees)

// ── Timing ──────────────────────────────────────────────────
unsigned long lastRead   = 0;
const int     READ_INTERVAL = 100;  // 10 Hz — comfortable for serial reading

// ── Previous Gesture (avoid spamming same output) ───────────
String lastGesture = "";
unsigned long lastGestureTime = 0;
const int     GESTURE_HOLD_MS = 500; // Must hold gesture 500 ms to confirm

// ── Finger Names (for debug prints) ────────────────────────
const char* fingerName[5] = { "Thumb", "Index", "Middle", "Ring", "Pinky" };

// ══════════════════════════════════════════════════════════════
// SETUP
// ══════════════════════════════════════════════════════════════

void setup() {
  Serial.begin(115200);

  // Wait for native USB serial (ESP32-S3 USB-CDC needs this)
  unsigned long start = millis();
  while (!Serial && (millis() - start < 3000));

  Serial.println();
  Serial.println("═══════════════════════════════════════");
  Serial.println("  Project Ability — Glove-to-Language  ");
  Serial.println("═══════════════════════════════════════");
  Serial.println();

  // ── Configure ADC ─────────────────────────────────────────
  analogReadResolution(12);
  analogSetAttenuation(ADC_11db);  // 0–3.1 V range

  // Quick-test each flex pin (millivolts)
  int pins[5] = { FLEX_THUMB, FLEX_INDEX, FLEX_MIDDLE, FLEX_RING, FLEX_PINKY };
  Serial.println("[FLEX] Testing analog pins (millivolts)...");
  for (int i = 0; i < 5; i++) {
    uint32_t mv = analogReadMilliVolts(pins[i]);
    Serial.printf("  GPIO %d (%s): %d mV\n", pins[i], fingerName[i], mv);
  }

  // ── I²C for BNO055 ───────────────────────────────────────
  Wire.begin(IMU_SDA, IMU_SCL);
  Wire.setClock(100000);   // 100 kHz — safer for startup
  Wire.setTimeOut(50);     // 50 ms timeout — prevents crash if no device

  // Scan I2C bus for BNO055 at address 0x28 before calling begin()
  Serial.println("\n[IMU]  Scanning I2C for BNO055 at 0x28...");
  Wire.beginTransmission(0x28);
  uint8_t i2cError = Wire.endTransmission();

  if (i2cError == 0) {
    Serial.println("[IMU]  Device found at 0x28 — initialising...");
    if (bno.begin()) {
      bno.setExtCrystalUse(true);
      bnoReady = true;
      Serial.println("[IMU]  OK — BNO055 ready");
    } else {
      Serial.println("[IMU]  begin() failed — device present but not responding");
    }
  } else {
    Serial.printf("[IMU]  No device at 0x28 (I2C error %d)\n", i2cError);
    Serial.println("[IMU]  Skipping IMU — flex sensors only");
  }

  // ── AUTO-CALIBRATION (10 seconds) ─────────────────────────
  // Phase 1: 5 seconds — keep hand FLAT/STRAIGHT
  // Phase 2: 5 seconds — BEND all fingers fully
  // The code records min and max millivolt readings per finger.

  int minMV[5], maxMV[5];
  for (int i = 0; i < 5; i++) {
    minMV[i] = 9999;
    maxMV[i] = 0;
  }

  Serial.println("\n╔═══════════════════════════════════════╗");
  Serial.println("║  AUTO-CALIBRATION — 10 seconds total  ║");
  Serial.println("╠═══════════════════════════════════════╣");
  Serial.println("║  First 5s: Keep hand FLAT / STRAIGHT  ║");
  Serial.println("║  Last  5s: BEND all fingers fully     ║");
  Serial.println("╚═══════════════════════════════════════╝\n");

  for (int t = 0; t < 100; t++) {  // 100 × 100ms = 10 seconds
    int sec = (t * 100) / 1000;
    if (t == 0)  Serial.println(">>> KEEP HAND FLAT NOW <<<");
    if (t == 50) Serial.println("\n>>> BEND ALL FINGERS NOW <<<");

    Serial.printf("[CAL %2ds] ", sec);
    for (int i = 0; i < 5; i++) {
      uint32_t mv = analogReadMilliVolts(pins[i]);
      if ((int)mv < minMV[i]) minMV[i] = (int)mv;
      if ((int)mv > maxMV[i]) maxMV[i] = (int)mv;
      Serial.printf("%s:%4d  ", fingerName[i], mv);
    }
    Serial.println();
    delay(100);
  }

  // Determine which direction flex sensors go
  // (some go high→low when bent, others low→high)
  // We sample once flat to determine direction
  Serial.println("\n--- Calibration Results (millivolts) ---");
  for (int i = 0; i < 5; i++) {
    FLEX_STRAIGHT[i] = maxMV[i];  // Assume straight = higher voltage
    FLEX_BENT[i]     = minMV[i];  // Assume bent = lower voltage
    Serial.printf("  %s: STRAIGHT=%d  BENT=%d  (range=%d mV)\n",
      fingerName[i], FLEX_STRAIGHT[i], FLEX_BENT[i],
      abs(FLEX_STRAIGHT[i] - FLEX_BENT[i]));

    // Warn if range is too small (bad wiring or no sensor)
    if (abs(FLEX_STRAIGHT[i] - FLEX_BENT[i]) < 50) {
      Serial.printf("  ⚠ WARNING: %s has very small range — check wiring!\n",
        fingerName[i]);
    }
  }
  calibrated = true;

  Serial.println("\n───────────────────────────────────────");
  Serial.println("  Calibration done — running live now   ");
  Serial.println("───────────────────────────────────────\n");
}

// ══════════════════════════════════════════════════════════════
// MAIN LOOP
// ══════════════════════════════════════════════════════════════

void loop() {
  unsigned long now = millis();
  if (now - lastRead < READ_INTERVAL) return;
  lastRead = now;

  // ── 1. Read flex sensors ──────────────────────────────────
  readFlexSensors();

  // ── 2. Read IMU orientation ───────────────────────────────
  readIMU();

  // ── 3. Classify gesture ───────────────────────────────────
  String gesture = classifyGesture();

  // ── 4. Print result (with debounce) ───────────────────────
  if (gesture != "---") {
    if (gesture == lastGesture) {
      // Gesture held long enough → confirm and print
      if (now - lastGestureTime >= GESTURE_HOLD_MS) {
        Serial.println(">>> " + gesture + " <<<");
        lastGestureTime = now;  // Reset so it doesn't spam
      }
    } else {
      // New gesture detected — start hold timer
      lastGesture = gesture;
      lastGestureTime = now;
    }
  }

  // ── 5. Print raw debug data every cycle ───────────────────
  printDebugLine(gesture);
}

// ══════════════════════════════════════════════════════════════
// READ FLEX SENSORS
// ══════════════════════════════════════════════════════════════

void readFlexSensors() {
  int pins[5] = { FLEX_THUMB, FLEX_INDEX, FLEX_MIDDLE, FLEX_RING, FLEX_PINKY };

  for (int i = 0; i < 5; i++) {
    rawMV[i] = (int)analogReadMilliVolts(pins[i]);

    // Map to 0–100% bend (clamped)
    if (calibrated && FLEX_STRAIGHT[i] != FLEX_BENT[i]) {
      bend[i] = map(rawMV[i], FLEX_STRAIGHT[i], FLEX_BENT[i], 0, 100);
      bend[i] = constrain(bend[i], 0, 100);
    } else {
      bend[i] = 0;  // Not calibrated yet
    }
  }
}

// ══════════════════════════════════════════════════════════════
// READ IMU (BNO055 / SEN0253)
// ══════════════════════════════════════════════════════════════

void readIMU() {
  if (!bnoReady) return;

  sensors_event_t event;
  bno.getEvent(&event, Adafruit_BNO055::VECTOR_EULER);

  yaw   = event.orientation.x;   // 0–360°
  roll  = event.orientation.y;   // ±180°
  pitch = event.orientation.z;   // ±90°
}

// ══════════════════════════════════════════════════════════════
// GESTURE CLASSIFICATION
// ══════════════════════════════════════════════════════════════
//
// Finger bend thresholds:
//   OPEN  = bend <  30%  (finger is straight / extended)
//   BENT  = bend >= 60%  (finger is curled in)
//
// Orientation thresholds (Euler angles from BNO055):
//   PALM_UP   = pitch roughly  0° to  45°
//   PALM_DOWN = pitch roughly -45° to -90° (or 135° to 180°)
//   PALM_FWD  = roll  roughly  0° ± 30°
//
// The gesture table below covers basic ASL-like letters.
// Expand as needed for your project.
// ══════════════════════════════════════════════════════════════

// Helper macros
#define OPEN(f)   (bend[f] < 30)
#define BENT(f)   (bend[f] >= 60)
#define HALF(f)   (bend[f] >= 30 && bend[f] < 60)

// Finger indices
#define THUMB   0
#define INDEX   1
#define MIDDLE  2
#define RING    3
#define PINKY   4

String classifyGesture() {

  // ─────────── FIST (all fingers bent) ──────────────────────
  if (BENT(THUMB) && BENT(INDEX) && BENT(MIDDLE) && BENT(RING) && BENT(PINKY)) {
    return "S";  // ASL 'S' — fist
  }

  // ─────────── OPEN HAND (all fingers open) ─────────────────
  if (OPEN(THUMB) && OPEN(INDEX) && OPEN(MIDDLE) && OPEN(RING) && OPEN(PINKY)) {
    // Check palm orientation for different signs
    if (bnoReady) {
      if (pitch > -30 && pitch < 30) {
        return "B";  // ASL 'B' — flat hand, palm forward
      }
      if (pitch > 60 || pitch < -60) {
        return "5";  // ASL '5' — open hand, palm down/up
      }
    }
    return "B";  // Default: open hand = B
  }

  // ─────────── INDEX ONLY (pointing) ────────────────────────
  if (OPEN(INDEX) && BENT(MIDDLE) && BENT(RING) && BENT(PINKY)) {
    if (BENT(THUMB)) {
      return "D";  // ASL 'D' — index up, others curled, thumb touches middle
    }
    if (OPEN(THUMB)) {
      return "L";  // ASL 'L' — index + thumb open (L shape)
    }
  }

  // ─────────── PEACE SIGN (index + middle open) ─────────────
  if (OPEN(INDEX) && OPEN(MIDDLE) && BENT(RING) && BENT(PINKY)) {
    if (BENT(THUMB)) {
      return "V";  // ASL 'V' (or number 2)
    }
    if (OPEN(THUMB)) {
      return "W";  // ASL 'W' would be 3 fingers — but thumb+index+middle ≈ 'W' variant
    }
  }

  // ─────────── THREE FINGERS (index + middle + ring open) ───
  if (OPEN(INDEX) && OPEN(MIDDLE) && OPEN(RING) && BENT(PINKY)) {
    if (BENT(THUMB)) {
      return "W";  // ASL 'W' — three middle fingers up
    }
    if (OPEN(THUMB)) {
      return "4";  // ASL '4' — four fingers open, pinky bent (close enough)
    }
  }

  // ─────────── THUMB UP ONLY ────────────────────────────────
  if (OPEN(THUMB) && BENT(INDEX) && BENT(MIDDLE) && BENT(RING) && BENT(PINKY)) {
    return "A";  // ASL 'A' — fist with thumb up/beside
  }

  // ─────────── PINKY ONLY (I Love You pinky) ────────────────
  if (BENT(THUMB) && BENT(INDEX) && BENT(MIDDLE) && BENT(RING) && OPEN(PINKY)) {
    return "I";  // ASL 'I' — pinky up
  }

  // ─────────── THUMB + PINKY (hang loose / 'Y') ─────────────
  if (OPEN(THUMB) && BENT(INDEX) && BENT(MIDDLE) && BENT(RING) && OPEN(PINKY)) {
    return "Y";  // ASL 'Y' — thumb + pinky spread
  }

  // ─────────── THUMB + INDEX + PINKY (I Love You) ───────────
  if (OPEN(THUMB) && OPEN(INDEX) && BENT(MIDDLE) && BENT(RING) && OPEN(PINKY)) {
    return "I Love You";  // ASL 'ILY' sign
  }

  // ─────────── INDEX + PINKY (Rock / horns) ─────────────────
  if (BENT(THUMB) && OPEN(INDEX) && BENT(MIDDLE) && BENT(RING) && OPEN(PINKY)) {
    return "U";  // ASL 'U' variant / horns
  }

  // ─────────── THUMB + INDEX pinch (O shape) ────────────────
  if (HALF(THUMB) && HALF(INDEX) && BENT(MIDDLE) && BENT(RING) && BENT(PINKY)) {
    return "O";  // ASL 'O' — fingers curled into O shape
  }

  // ─────────── C shape (all fingers half-bent) ──────────────
  if (HALF(THUMB) && HALF(INDEX) && HALF(MIDDLE) && HALF(RING) && HALF(PINKY)) {
    return "C";  // ASL 'C' — cupped hand
  }

  // ─────────── INDEX + MIDDLE + PINKY open ──────────────────
  if (OPEN(INDEX) && OPEN(MIDDLE) && BENT(RING) && OPEN(PINKY)) {
    if (OPEN(THUMB)) {
      return "I Love You";  // Already covered above, but safety net
    }
  }

  // ─────────── RING + PINKY open, others bent ───────────────
  if (BENT(THUMB) && BENT(INDEX) && BENT(MIDDLE) && OPEN(RING) && OPEN(PINKY)) {
    return "6";  // Number 6 variant
  }

  // ─────────── NO MATCH ─────────────────────────────────────
  return "---";
}

// ══════════════════════════════════════════════════════════════
// DEBUG OUTPUT (one compact line per cycle)
// ══════════════════════════════════════════════════════════════

void printDebugLine(String gesture) {
  // Show millivolts + bend % + IMU + gesture
  Serial.printf("mV[%4d %4d %4d %4d %4d] ",
    rawMV[THUMB], rawMV[INDEX], rawMV[MIDDLE], rawMV[RING], rawMV[PINKY]);

  Serial.printf("BEND[T:%3d I:%3d M:%3d R:%3d P:%3d]", 
    bend[THUMB], bend[INDEX], bend[MIDDLE], bend[RING], bend[PINKY]);

  if (bnoReady) {
    Serial.printf(" | Pit:%6.1f Rol:%6.1f Yaw:%6.1f", pitch, roll, yaw);

    // Show calibration status periodically
    static unsigned long lastCalPrint = 0;
    if (millis() - lastCalPrint > 5000) {
      uint8_t sys, gyro, accel, mag;
      bno.getCalibration(&sys, &gyro, &accel, &mag);
      Serial.printf(" [Cal S:%d G:%d A:%d M:%d]", sys, gyro, accel, mag);
      lastCalPrint = millis();
    }
  }

  Serial.printf(" | %s\n", gesture.c_str());
}
