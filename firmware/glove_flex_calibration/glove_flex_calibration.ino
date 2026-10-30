/*
 * Project Ability — Simple Flex Sensor Calibration
 * Board: ESP32-S3 Super Mini
 *
 * Uses the same wiring and ADC settings as glove_flex_imu.ino:
 *   Thumb  = GPIO 1
 *   Index  = GPIO 2
 *   Middle = GPIO 3
 *   Ring   = GPIO 4
 *   Pinky  = GPIO 5
 *
 * Open Serial Monitor at 115200 baud and follow the instructions.
 * Send R at any time to run the calibration again.
 */

const uint8_t FLEX_PINS[5] = {1, 2, 3, 4, 5};
const char *FINGER_NAMES[5] = {
  "THUMB", "INDEX", "MIDDLE", "RING", "PINKY"
};

const unsigned long SAMPLE_TIME_MS = 2500;
const unsigned long SAMPLE_DELAY_MS = 10;
const int MIN_USEFUL_RANGE_MV = 50;

int flatMV[5] = {0, 0, 0, 0, 0};
int bentMV[5] = {0, 0, 0, 0, 0};
bool calibrated = false;


void countdown(const char *instruction) {
  Serial.println();
  Serial.println(instruction);
  for (int seconds = 3; seconds > 0; seconds--) {
    Serial.print("Starting in ");
    Serial.print(seconds);
    Serial.println("...");
    delay(1000);
  }
}


void capturePosition(int result[5]) {
  uint32_t totals[5] = {0, 0, 0, 0, 0};
  uint32_t samples = 0;
  unsigned long startedAt = millis();

  while (millis() - startedAt < SAMPLE_TIME_MS) {
    for (int finger = 0; finger < 5; finger++) {
      totals[finger] += analogReadMilliVolts(FLEX_PINS[finger]);
    }
    samples++;
    delay(SAMPLE_DELAY_MS);
  }

  for (int finger = 0; finger < 5; finger++) {
    result[finger] = totals[finger] / samples;
  }
}


int bendPercent(int rawMV, int flatValue, int bentValue) {
  if (abs(bentValue - flatValue) < MIN_USEFUL_RANGE_MV) {
    return 0;
  }

  // This works whether bending makes the voltage rise or fall.
  float percent = 100.0f * (rawMV - flatValue) / (bentValue - flatValue);
  return constrain((int)round(percent), 0, 100);
}


void printArray(const char *name, const int values[5]) {
  Serial.print("const int ");
  Serial.print(name);
  Serial.print("[5] = {");
  for (int finger = 0; finger < 5; finger++) {
    if (finger > 0) Serial.print(", ");
    Serial.print(values[finger]);
  }
  Serial.println("};");
}


void printCalibration() {
  Serial.println();
  Serial.println("========== CALIBRATION RESULT ==========");
  Serial.println("Copy these two lines into your glove program:");
  Serial.println();
  printArray("FLEX_FLAT_MV", flatMV);
  printArray("FLEX_BENT_MV", bentMV);
  Serial.println();

  bool allSensorsOK = true;
  for (int finger = 0; finger < 5; finger++) {
    int range = abs(bentMV[finger] - flatMV[finger]);
    Serial.print(FINGER_NAMES[finger]);
    Serial.print(": flat=");
    Serial.print(flatMV[finger]);
    Serial.print(" mV, bent=");
    Serial.print(bentMV[finger]);
    Serial.print(" mV, range=");
    Serial.print(range);
    Serial.print(" mV");
    if (range < MIN_USEFUL_RANGE_MV) {
      Serial.print("  <-- CHECK SENSOR/WIRING");
      allSensorsOK = false;
    }
    Serial.println();
  }

  Serial.println();
  if (allSensorsOK) {
    Serial.println("Calibration looks good.");
  } else {
    Serial.println("One or more sensors have too little change. Check the voltage divider.");
  }
  Serial.println("Live output: raw millivolts followed by bend percentage.");
  Serial.println("Send R to recalibrate.");
  Serial.println("========================================");
}


void runCalibration() {
  calibrated = false;

  countdown("STEP 1: Keep every finger completely STRAIGHT and relaxed.");
  Serial.println("Reading FLAT position...");
  capturePosition(flatMV);
  Serial.println("Flat position saved.");

  countdown("STEP 2: Make a comfortable CLOSED FIST. Bend every finger fully.");
  Serial.println("Reading BENT position...");
  capturePosition(bentMV);
  Serial.println("Bent position saved.");

  calibrated = true;
  printCalibration();
}


void setup() {
  Serial.begin(115200);
  unsigned long serialWaitStarted = millis();
  while (!Serial && millis() - serialWaitStarted < 3000) {
    delay(10);
  }

  analogReadResolution(12);
  analogSetAttenuation(ADC_11db);

  Serial.println();
  Serial.println("PROJECT ABILITY — FLEX SENSOR CALIBRATION");
  Serial.println("GPIO order: Thumb=1, Index=2, Middle=3, Ring=4, Pinky=5");
  runCalibration();
}


void loop() {
  if (Serial.available()) {
    char command = Serial.read();
    if (command == 'r' || command == 'R') {
      runCalibration();
      return;
    }
  }

  if (!calibrated) return;

  Serial.print("RAW mV [");
  int rawValues[5];
  for (int finger = 0; finger < 5; finger++) {
    rawValues[finger] = analogReadMilliVolts(FLEX_PINS[finger]);
    if (finger > 0) Serial.print(", ");
    Serial.print(rawValues[finger]);
  }

  Serial.print("]  BEND % [");
  for (int finger = 0; finger < 5; finger++) {
    if (finger > 0) Serial.print(", ");
    Serial.print(bendPercent(rawValues[finger], flatMV[finger], bentMV[finger]));
  }
  Serial.println("]");

  delay(150);
}
