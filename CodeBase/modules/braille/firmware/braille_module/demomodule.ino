#include <ESP32Servo.h>

#define M1_LEFT_PIN   13
#define M1_RIGHT_PIN  12
#define M2_LEFT_PIN   14
#define M2_RIGHT_PIN  27
#define M3_LEFT_PIN   26
#define M3_RIGHT_PIN  25
#define M4_LEFT_PIN   33
#define M4_RIGHT_PIN  32

#define NUM_MODULES 4

#define SERVO_MIN_US 500
#define SERVO_MAX_US 2400

#define CHAR_HOLD_TIME  5000
#define BATCH_GAP_TIME  3000
#define SERVO_MOVE_TIME 300
#define SERVO_SETTLE_MS 30

const int CAM_ANGLES[8] = {
    32, 66, 180, 88, 154, 152, 132, 105
};

const int RIGHT_CAM_ANGLES[8] = {
    22, 44, 170, 66, 148, 152, 132, 105
};

uint8_t charToBraille(char c) {
  c = tolower(c);
  switch (c) {
  case 'a': return 0b000001;
  case 'b': return 0b000011;
  case 'c': return 0b001001;
  case 'd': return 0b011001;
  case 'e': return 0b010001;
  case 'f': return 0b001011;
  case 'g': return 0b011011;
  case 'h': return 0b010011;
  case 'i': return 0b001010;
  case 'j': return 0b011010;
  case 'k': return 0b000101;
  case 'l': return 0b000111;
  case 'm': return 0b001101;
  case 'n': return 0b011101;
  case 'o': return 0b010101;
  case 'p': return 0b001111;
  case 'q': return 0b011111;
  case 'r': return 0b010111;
  case 's': return 0b001110;
  case 't': return 0b011110;
  case 'u': return 0b100101;
  case 'v': return 0b100111;
  case 'w': return 0b111010;
  case 'x': return 0b101101;
  case 'y': return 0b111101;
  case 'z': return 0b110101;
  case ' ': return 0b000000;
  default:  return 0xFF;
  }
}

Servo m1Left;
Servo m1Right;
Servo m2Left;
Servo m2Right;
Servo m3Left;
Servo m3Right;
Servo m4Left;
Servo m4Right;

bool moduleOK[NUM_MODULES] = { false, false, false, false };

String inputBuffer = "";

void writeLeftServo(int module, int angle) {
  switch (module) {
    case 0: m1Left.write(angle); break;
    case 1: m2Left.write(angle); break;
    case 2: m3Left.write(angle); break;
    case 3: m4Left.write(angle); break;
  }
}

void writeRightServo(int module, int angle) {
  switch (module) {
    case 0: m1Right.write(angle); break;
    case 1: m2Right.write(angle); break;
    case 2: m3Right.write(angle); break;
    case 3: m4Right.write(angle); break;
  }
}

void driveModule(int module, uint8_t pattern) {
  uint8_t leftPattern = pattern & 0x07;
  uint8_t rightPattern = (pattern >> 3) & 0x07;

  rightPattern = ((rightPattern & 0x01) << 2)
                 | (rightPattern & 0x02)
                 | ((rightPattern & 0x04) >> 2);

  int leftAngle = CAM_ANGLES[leftPattern];
  int rightAngle = RIGHT_CAM_ANGLES[rightPattern];

  writeLeftServo(module, leftAngle);
  delay(SERVO_SETTLE_MS);
  writeRightServo(module, rightAngle);
  delay(SERVO_SETTLE_MS);

  Serial.print("    M");
  Serial.print(module + 1);
  Serial.print(": L=");
  Serial.print(leftAngle);
  Serial.print(" R=");
  Serial.print(rightAngle);
  Serial.println("");
}

void homeModule(int module) {
  writeLeftServo(module, 22);
  delay(SERVO_SETTLE_MS);
  writeRightServo(module, 22);
  delay(SERVO_SETTLE_MS);
}

void homeAllServos() {
  for (int m = 0; m < NUM_MODULES; m++) {
    if (moduleOK[m]) {
      homeModule(m);
    }
  }
  delay(SERVO_MOVE_TIME);
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println();
  Serial.println("BRAILLE 4-MODULE DISPLAY");
  Serial.println("Servos: 8x MG90S (2 per module x 4 modules)");
  Serial.println();

  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);

  Serial.print("[Module 1] GPIO 13 + GPIO 12... ");
  m1Left.setPeriodHertz(50);
  m1Left.attach(M1_LEFT_PIN, SERVO_MIN_US, SERVO_MAX_US);
  delay(100);
  m1Right.setPeriodHertz(50);
  m1Right.attach(M1_RIGHT_PIN, SERVO_MIN_US, SERVO_MAX_US);
  delay(100);
  moduleOK[0] = m1Left.attached() && m1Right.attached();
  Serial.println(moduleOK[0] ? "OK" : "FAILED");

  Serial.print("[Module 2] GPIO 14 + GPIO 27... ");
  m2Left.setPeriodHertz(50);
  m2Left.attach(M2_LEFT_PIN, SERVO_MIN_US, SERVO_MAX_US);
  delay(100);
  m2Right.setPeriodHertz(50);
  m2Right.attach(M2_RIGHT_PIN, SERVO_MIN_US, SERVO_MAX_US);
  delay(100);
  moduleOK[1] = m2Left.attached() && m2Right.attached();
  Serial.println(moduleOK[1] ? "OK" : "FAILED");

  Serial.print("[Module 3] GPIO 26 + GPIO 25... ");
  m3Left.setPeriodHertz(50);
  m3Left.attach(M3_LEFT_PIN, SERVO_MIN_US, SERVO_MAX_US);
  delay(100);
  m3Right.setPeriodHertz(50);
  m3Right.attach(M3_RIGHT_PIN, SERVO_MIN_US, SERVO_MAX_US);
  delay(100);
  moduleOK[2] = m3Left.attached() && m3Right.attached();
  Serial.println(moduleOK[2] ? "OK" : "FAILED");

  Serial.print("[Module 4] GPIO 33 + GPIO 32... ");
  m4Left.setPeriodHertz(50);
  m4Left.attach(M4_LEFT_PIN, SERVO_MIN_US, SERVO_MAX_US);
  delay(100);
  m4Right.setPeriodHertz(50);
  m4Right.attach(M4_RIGHT_PIN, SERVO_MIN_US, SERVO_MAX_US);
  delay(100);
  moduleOK[3] = m4Left.attached() && m4Right.attached();
  Serial.println(moduleOK[3] ? "OK" : "FAILED");

  int okCount = 0;
  for (int m = 0; m < NUM_MODULES; m++) {
    if (moduleOK[m]) okCount++;
  }

  Serial.println();
  Serial.print(okCount);
  Serial.println("/4 modules attached.");

  Serial.println();
  Serial.println("Moving all servos to home (22)...");
  homeAllServos();
  delay(1000);
  Serial.println("All servos at home.");

  Serial.println();
  Serial.println("Module 1: Left=GPIO13, Right=GPIO12");
  Serial.println("Module 2: Left=GPIO14, Right=GPIO27");
  Serial.println("Module 3: Left=GPIO26, Right=GPIO25");
  Serial.println("Module 4: Left=GPIO33, Right=GPIO32");

  Serial.println();
  Serial.println("READY! Type a word to display.");
  Serial.println("Commands: test, home, sweep, diag");
  Serial.println();
}

void loop() {
  if (Serial.available() > 0) {
    inputBuffer = Serial.readStringUntil('\n');
    inputBuffer.trim();

    if (inputBuffer.length() > 0) {
      Serial.println();
      Serial.print("Received: ");
      Serial.println(inputBuffer);
      Serial.println();

      if (inputBuffer.equalsIgnoreCase("test")) {
        runTestAllLetters();
      } else if (inputBuffer.equalsIgnoreCase("home")) {
        homeAllServos();
        Serial.println("All servos at 22.");
      } else if (inputBuffer.equalsIgnoreCase("sweep")) {
        runSweepTest();
      } else if (inputBuffer.equalsIgnoreCase("diag")) {
        runDiagnostics();
      } else {
        displayWord(inputBuffer);
      }

      Serial.println();
      Serial.println("Ready for next input...");
      Serial.println();
    }
  }
}

void displayWord(String word) {
  int totalChars = word.length();
  int totalBatches = (totalChars + NUM_MODULES - 1) / NUM_MODULES;

  Serial.print("Word: ");
  Serial.print(word);
  Serial.print(" -> ");
  Serial.print(totalChars);
  Serial.print(" chars, ");
  Serial.print(totalBatches);
  Serial.println(" batch(es)");
  Serial.println();

  for (int batch = 0; batch < totalBatches; batch++) {
    int startIdx = batch * NUM_MODULES;
    int charsInBatch = min(NUM_MODULES, totalChars - startIdx);

    Serial.print("Batch ");
    Serial.print(batch + 1);
    Serial.print("/");
    Serial.println(totalBatches);

    uint8_t patterns[NUM_MODULES];
    bool    active[NUM_MODULES];

    for (int m = 0; m < NUM_MODULES; m++) {
      if (m < charsInBatch) {
        char c = word.charAt(startIdx + m);
        patterns[m] = charToBraille(c);
        active[m] = (patterns[m] != 0xFF) && moduleOK[m];

        Serial.print("  Module ");
        Serial.print(m + 1);
        Serial.print(": '");
        Serial.print(c);
        Serial.print("' -> ");

        if (!moduleOK[m]) {
          Serial.println("(not attached)");
        } else if (patterns[m] == 0xFF) {
          Serial.println("(unsupported)");
        } else {
          printBrailleInfo(c, patterns[m]);
        }
      } else {
        patterns[m] = 0b000000;
        active[m] = moduleOK[m];
      }
    }

    Serial.println();
    Serial.println("  Driving modules...");

    for (int m = 0; m < NUM_MODULES; m++) {
      if (active[m]) {
        driveModule(m, patterns[m]);
      }
    }

    delay(SERVO_MOVE_TIME);

    Serial.print("  Holding ");
    Serial.print(CHAR_HOLD_TIME / 1000);
    Serial.println(" seconds...");
    delay(CHAR_HOLD_TIME);

    Serial.println("  Returning to home...");
    homeAllServos();

    if (batch < totalBatches - 1) {
      Serial.print("  Pause ");
      Serial.print(BATCH_GAP_TIME / 1000);
      Serial.println(" seconds...");
      delay(BATCH_GAP_TIME);
    }

    Serial.println();
  }

  Serial.println("Word complete!");
}

void printBrailleInfo(char c, uint8_t pattern) {
  Serial.print("0b");
  for (int b = 5; b >= 0; b--) {
    Serial.print((pattern >> b) & 1);
  }

  Serial.print("  Dots: ");
  bool first = true;
  for (int d = 0; d < 6; d++) {
    if (pattern & (1 << d)) {
      if (!first)
        Serial.print(",");
      Serial.print(d + 1);
      first = false;
    }
  }
  if (first)
    Serial.print("(none)");

  Serial.print("  Cell: ");
  Serial.print((pattern & 0x01) ? "O" : ".");
  Serial.print((pattern & 0x08) ? "O" : ".");
  Serial.print(" ");
  Serial.print((pattern & 0x02) ? "O" : ".");
  Serial.print((pattern & 0x10) ? "O" : ".");
  Serial.print(" ");
  Serial.print((pattern & 0x04) ? "O" : ".");
  Serial.println((pattern & 0x20) ? "O" : ".");
}

void runTestAllLetters() {
  Serial.println("Running a-z in groups of 4...");
  Serial.println();

  for (int i = 0; i < 26; i += NUM_MODULES) {
    int charsInBatch = min(NUM_MODULES, 26 - i);

    for (int m = 0; m < charsInBatch; m++) {
      char c = 'a' + i + m;
      uint8_t pattern = charToBraille(c);

      Serial.print("  M");
      Serial.print(m + 1);
      Serial.print(": ");
      Serial.print(c);
      Serial.print(" -> ");
      printBrailleInfo(c, pattern);

      if (moduleOK[m]) {
        driveModule(m, pattern);
      }
    }

    delay(SERVO_MOVE_TIME);
    Serial.print("  Holding 5 seconds...");
    delay(CHAR_HOLD_TIME);
    Serial.println(" done.");

    homeAllServos();
    delay(BATCH_GAP_TIME);
    Serial.println();
  }

  Serial.println("All 26 letters complete!");
}

void runDiagnostics() {
  Serial.println("Testing each module individually...");
  Serial.println();

  for (int m = 0; m < NUM_MODULES; m++) {
    Serial.print("Module ");
    Serial.print(m + 1);

    if (!moduleOK[m]) {
      Serial.println(" SKIPPED (not attached)");
      continue;
    }

    Serial.println(":");

    Serial.print("  Left -> 90... ");
    writeLeftServo(m, 90);
    delay(500);
    Serial.println("done.");

    Serial.print("  Left -> 22... ");
    writeLeftServo(m, 22);
    delay(500);
    Serial.println("done.");

    Serial.print("  Right -> 90... ");
    writeRightServo(m, 90);
    delay(500);
    Serial.println("done.");

    Serial.print("  Right -> 22... ");
    writeRightServo(m, 22);
    delay(500);
    Serial.println("done.");

    Serial.print("  Braille 'a' test... ");
    driveModule(m, charToBraille('a'));
    delay(2000);
    homeModule(m);
    delay(300);
    Serial.println("done.");

    Serial.println();
  }

  Serial.println("Diagnostics complete.");
}

void runSweepTest() {
  Serial.println("Sweeping all servos...");
  Serial.println();

  for (int m = 0; m < NUM_MODULES; m++) {
    if (!moduleOK[m]) {
      Serial.print("Module ");
      Serial.print(m + 1);
      Serial.println(" SKIPPED (not attached)");
      continue;
    }

    Serial.print("Module ");
    Serial.print(m + 1);
    Serial.println(":");

    Serial.println("  Left: 0 -> 180");
    for (int angle = 0; angle <= 180; angle += 10) {
      writeLeftServo(m, angle);
      delay(150);
    }
    for (int angle = 180; angle >= 0; angle -= 10) {
      writeLeftServo(m, angle);
      delay(150);
    }
    Serial.println("  Left done.");

    Serial.println("  Right: 0 -> 180");
    for (int angle = 0; angle <= 180; angle += 10) {
      writeRightServo(m, angle);
      delay(150);
    }
    for (int angle = 180; angle >= 0; angle -= 10) {
      writeRightServo(m, angle);
      delay(150);
    }
    Serial.println("  Right done.");
    Serial.println();
  }

  homeAllServos();
  Serial.println("Sweep complete. All servos at 22.");
}
