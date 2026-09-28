/*
 * GLOVE-TO-LANGUAGE — Full ASL + Working Calibration
 * ESP32-S3 Super Mini
 * 
 * GPIO 1-5: Flex sensors (with 10k voltage dividers)
 * GPIO 8: SEN0253 SDA   GPIO 9: SEN0253 SCL
 *
 * No external libraries — raw Wire for BNO055.
 */

#include <Wire.h>

// Pins
const int FLEX_PIN[5] = {1, 2, 3, 4, 5};
const char* FNAME[5] = {"Th", "Ix", "Md", "Rg", "Pk"};

// BNO055 raw I2C
#define BNO_ADDR  0x28
#define BNO_CHIP  0x00
#define BNO_OPR   0x3D
#define BNO_PWR   0x3E
#define BNO_TRIG  0x3F
#define BNO_EULER 0x1A

// Finger indices
#define TH 0
#define IX 1
#define MD 2
#define RG 3
#define PK 4

// State
bool imuOK = false;
int rawMV[5];
int bend[5];
int calHi[5] = {0, 0, 0, 0, 0};
int calLo[5] = {3300, 3300, 3300, 3300, 3300};
bool autoCalDone = false;
unsigned long autoCalStart = 0;

// IMU
float heading = 0, roll_a = 0, pitch_a = 0;

// Gesture state
String lastG = "";
String confirmedG = "";
unsigned long lastGT = 0;
int gCount = 0;

// Sentence builder
String sentence = "";

// BNO055 helpers
uint8_t bnoRd(uint8_t reg) {
  Wire.beginTransmission(BNO_ADDR);
  Wire.write(reg);
  Wire.endTransmission(false);
  Wire.requestFrom((uint8_t)BNO_ADDR, (uint8_t)1);
  return Wire.available() ? Wire.read() : 0;
}

void bnoWr(uint8_t reg, uint8_t val) {
  Wire.beginTransmission(BNO_ADDR);
  Wire.write(reg);
  Wire.write(val);
  Wire.endTransmission();
}

// ═══════════════════════════════════════
// SETUP
// ═══════════════════════════════════════

void setup() {
  Serial.begin(115200);
  unsigned long s = millis();
  while (!Serial && (millis() - s < 3000));

  analogReadResolution(12);
  analogSetAttenuation(ADC_11db);

  Serial.println("\n--- Glove-to-Language ---");

  for (int i = 0; i < 5; i++) {
    Serial.print(FNAME[i]);
    Serial.print(":");
    Serial.print(analogReadMilliVolts(FLEX_PIN[i]));
    Serial.print("mV  ");
  }
  Serial.println();

  // BNO055
  Wire.begin(8, 9);
  Wire.setClock(100000);
  delay(100);

  Wire.beginTransmission(BNO_ADDR);
  if (Wire.endTransmission() == 0) {
    uint8_t id = bnoRd(BNO_CHIP);
    if (id == 0xA0) {
      bnoWr(BNO_TRIG, 0x20);
      delay(700);
      bnoWr(BNO_PWR, 0x00);
      delay(10);
      bnoWr(BNO_OPR, 0x0C);
      delay(20);
      imuOK = true;
      Serial.println("BNO055 OK");
    }
  } else {
    Serial.println("No BNO055");
  }

  Serial.println("Move fingers flat<->bent to calibrate\n");
  autoCalStart = millis();
}

// ═══════════════════════════════════════
// LOOP — same structure as working version
// ═══════════════════════════════════════

void loop() {
  // Read flex
  for (int i = 0; i < 5; i++) {
    rawMV[i] = analogReadMilliVolts(FLEX_PIN[i]);

    if (rawMV[i] > calHi[i]) calHi[i] = rawMV[i];
    if (rawMV[i] < calLo[i]) calLo[i] = rawMV[i];

    if (calHi[i] != calLo[i]) {
      bend[i] = map(rawMV[i], calHi[i], calLo[i], 0, 100);
      bend[i] = constrain(bend[i], 0, 100);
    }
  }

  // Cal snapshot once
  if (!autoCalDone && millis() - autoCalStart > 10000) {
    autoCalDone = true;
    Serial.println("\n--- Calibration ---");
    for (int i = 0; i < 5; i++) {
      Serial.print(FNAME[i]);
      Serial.print(":");
      Serial.print(calLo[i]);
      Serial.print("-");
      Serial.print(calHi[i]);
      Serial.print("  ");
    }
    Serial.println("\n");
  }

  // Read IMU
  if (imuOK) {
    Wire.beginTransmission(BNO_ADDR);
    Wire.write(BNO_EULER);
    Wire.endTransmission(false);
    Wire.requestFrom((uint8_t)BNO_ADDR, (uint8_t)6);
    if (Wire.available() >= 6) {
      uint8_t b[6];
      for (int i = 0; i < 6; i++) b[i] = Wire.read();
      heading = (int16_t)(b[1] << 8 | b[0]) / 16.0;
      roll_a  = (int16_t)(b[3] << 8 | b[2]) / 16.0;
      pitch_a = (int16_t)(b[5] << 8 | b[4]) / 16.0;
    }
  }

  // Classify
  String g = classify();

  // Gesture confirmation (hold 4 cycles = ~400ms)
  if (g != "---") {
    if (g == lastG) {
      gCount++;
      if (gCount == 4 && g != confirmedG) {
        confirmedG = g;
        sentence += g;
        Serial.println();
        Serial.print("==> ");
        Serial.print(g);
        Serial.println(" <==");
        Serial.print("Sentence: [");
        Serial.print(sentence);
        Serial.println("]");
        Serial.println();
      }
    } else {
      lastG = g;
      gCount = 0;
    }
  } else {
    gCount = 0;
    confirmedG = "";
  }

  // Debug — same format as working version
  Serial.print("mV[");
  for (int i = 0; i < 5; i++) {
    Serial.print(" ");
    Serial.print(rawMV[i]);
  }
  Serial.print("] BEND[T:");
  Serial.print(bend[TH]);
  Serial.print(" I:");
  Serial.print(bend[IX]);
  Serial.print(" M:");
  Serial.print(bend[MD]);
  Serial.print(" R:");
  Serial.print(bend[RG]);
  Serial.print(" P:");
  Serial.print(bend[PK]);
  Serial.print("]");
  if (imuOK) {
    Serial.print(" | Pit: ");
    Serial.print(pitch_a, 1);
    Serial.print(" Rol: ");
    Serial.print(roll_a, 1);
    Serial.print(" Yaw: ");
    Serial.print(heading, 1);
  }
  Serial.print(" | ");
  Serial.println(g);

  delay(100);
}

// ═══════════════════════════════════════
// CLASSIFY — Full ASL A-Z + 0-9
// ═══════════════════════════════════════
// Uses same thresholds that worked before:
//   OPEN  = bend < 30
//   HALF  = 30 to 59
//   BENT  = bend >= 60

String classify() {
  bool o[5], h[5], b[5];
  for (int i = 0; i < 5; i++) {
    o[i] = bend[i] < 30;
    h[i] = bend[i] >= 30 && bend[i] < 60;
    b[i] = bend[i] >= 60;
  }

  // Palm orientation (from IMU)
  bool palmFwd = true, palmDown = false, palmUp = false, palmSide = false;
  if (imuOK) {
    palmFwd  = pitch_a > -30 && pitch_a < 30;
    palmDown = pitch_a <= -30;
    palmUp   = pitch_a >= 60;
    palmSide = (roll_a > 40 && roll_a < 140) || (roll_a < -40 && roll_a > -140);
  }

  // === ALL BENT (fist) ===
  if (b[TH]&&b[IX]&&b[MD]&&b[RG]&&b[PK]) return "S";

  // === ALL OPEN ===
  if (o[TH]&&o[IX]&&o[MD]&&o[RG]&&o[PK]) {
    if (imuOK && (palmDown||palmUp)) return "5";
    return "B";
  }

  // === THUMB ONLY OPEN ===
  if (o[TH]&&b[IX]&&b[MD]&&b[RG]&&b[PK]) return "A";

  // === INDEX ONLY OPEN ===
  if (b[TH]&&o[IX]&&b[MD]&&b[RG]&&b[PK]) {
    if (imuOK && palmSide) return "G";
    if (imuOK && palmDown) return "Q";
    return "D";
  }

  // === INDEX HALF (hooked) ===
  if (b[TH]&&h[IX]&&b[MD]&&b[RG]&&b[PK]) return "X";

  // === THUMB + INDEX OPEN ===
  if (o[TH]&&o[IX]&&b[MD]&&b[RG]&&b[PK]) return "L";

  // === INDEX + MIDDLE OPEN ===
  if (b[TH]&&o[IX]&&o[MD]&&b[RG]&&b[PK]) {
    if (imuOK && palmSide) return "H";
    if (imuOK && palmDown) return "P";
    return "V";
  }

  // === THUMB + INDEX + MIDDLE OPEN ===
  if (o[TH]&&o[IX]&&o[MD]&&b[RG]&&b[PK]) return "3";

  // === INDEX + MIDDLE OPEN, THUMB HALF ===
  if (h[TH]&&o[IX]&&o[MD]&&b[RG]&&b[PK]) return "K";

  // === INDEX + MIDDLE + RING OPEN ===
  if (b[TH]&&o[IX]&&o[MD]&&o[RG]&&b[PK]) return "W";

  // === 4 FINGERS OPEN (no thumb) ===
  if (b[TH]&&o[IX]&&o[MD]&&o[RG]&&o[PK]) return "9";

  // === 4 FINGERS OPEN (with thumb) ===
  if (o[TH]&&o[IX]&&o[MD]&&o[RG]&&b[PK]) return "4";

  // === PINKY ONLY ===
  if (b[TH]&&b[IX]&&b[MD]&&b[RG]&&o[PK]) return "I";

  // === THUMB + PINKY ===
  if (o[TH]&&b[IX]&&b[MD]&&b[RG]&&o[PK]) return "Y";

  // === THUMB + INDEX + PINKY (I Love You) ===
  if (o[TH]&&o[IX]&&b[MD]&&b[RG]&&o[PK]) return "ILY";

  // === INDEX + PINKY ===
  if (b[TH]&&o[IX]&&b[MD]&&b[RG]&&o[PK]) return "U";

  // === RING + PINKY OPEN ===
  if (b[TH]&&b[IX]&&b[MD]&&o[RG]&&o[PK]) return "6";

  // === MIDDLE ONLY OPEN ===
  if (b[TH]&&b[IX]&&o[MD]&&b[RG]&&b[PK]) return "8";

  // === MIDDLE + RING + PINKY OPEN ===
  if (b[TH]&&b[IX]&&o[MD]&&o[RG]&&o[PK]) return "7";

  // === ALL HALF (cupped) ===
  if (h[TH]&&h[IX]&&h[MD]&&h[RG]&&h[PK]) return "C";

  // === THUMB+INDEX HALF, REST BENT ===
  if (h[TH]&&h[IX]&&b[MD]&&b[RG]&&b[PK]) return "O";

  // === 4 HALF + PINKY BENT ===
  if (h[TH]&&h[IX]&&h[MD]&&h[RG]&&b[PK]) return "E";

  // === THUMB+INDEX HALF, 3 OPEN ===
  if (h[TH]&&h[IX]&&o[MD]&&o[RG]&&o[PK]) return "F";

  // === INDEX+MIDDLE HALF, REST BENT ===
  if (b[TH]&&h[IX]&&h[MD]&&b[RG]&&b[PK]) return "R";

  // === THUMB HALF, REST BENT ===
  if (h[TH]&&b[IX]&&b[MD]&&b[RG]&&b[PK]) return "T";

  // === THUMB HALF, INDEX OPEN, REST BENT ===
  if (h[TH]&&o[IX]&&b[MD]&&b[RG]&&b[PK]) return "1";

  // === THUMB+INDEX HALF, MIDDLE OPEN ===
  if (h[TH]&&h[IX]&&o[MD]&&b[RG]&&b[PK]) return "N";

  // === THUMB HALF, 3 FINGERS OPEN ===
  if (h[TH]&&o[IX]&&o[MD]&&o[RG]&&b[PK]) return "M";

  return "---";
}
