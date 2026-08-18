/*
 * ══════════════════════════════════════════════════════════════
 * BRAILLE MODULE v2 — Serial Monitor Test Firmware
 * Target: ESP32 (any variant — ESP32, ESP32-S3, etc.)
 * Servos: 2× MG90S (Metal Gear Micro Servo)
 * ══════════════════════════════════════════════════════════════
 *
 * HOW IT WORKS:
 *   1. On power-up, both servos initialize to 90° (HOME position).
 *      The rack-and-pinion is centered at 90° — this is the
 *      starting/neutral position where NO dots are raised.
 *   2. Open Serial Monitor at 115200 baud.
 *   3. Type a word (e.g. "hello") and press Enter.
 *   4. The firmware converts each letter to its Braille 6-bit
 *      pattern, splits into left column (dots 1,2,3) and right
 *      column (dots 4,5,6), looks up the cam angle for each
 *      column, and drives the two MG90S servos accordingly.
 *   5. Each letter is displayed for 2 seconds, then servos
 *      return to 90° (home) before the next letter.
 *
 * MECHANICAL DESIGN:
 *   The servo (MG90S) drives a spur gear (30 teeth) which
 *   meshes with the rack. At 90°, the rack is centered
 *   (starting position). The servo rotates from 90° outward
 *   to slide the rack to one of 8 cam positions.
 *
 * SERVOS:
 *   LEFT  MG90S → GPIO 13 → controls Dots 1, 2, 3 (left column)
 *   RIGHT MG90S → GPIO 12 → controls Dots 4, 5, 6 (right column)
 *
 * WIRING — see README.md for full wiring diagram.
 *
 * LIBRARIES REQUIRED (Arduino IDE Library Manager):
 *   - ESP32Servo
 */

#include <ESP32Servo.h>

// ── Servo Pin Definitions ───────────────────────────────────
// Change these if you wire to different GPIO pins
#define LEFT_SERVO_PIN   13   // MG90S — controls left rack → Dots 1,2,3
#define RIGHT_SERVO_PIN  12   // MG90S — controls right rack → Dots 4,5,6

// ── Servo Pulse Width (microseconds) for MG90S ─────────────
// MG90S typical range: 500µs (0°) to 2400µs (180°)
#define SERVO_MIN_US  500
#define SERVO_MAX_US  2400

// ── Home Position ──────────────────────────────────────────
// 90° = center/neutral = rack in starting position = no dots raised
#define HOME_ANGLE  90

// ── Timing (milliseconds) ──────────────────────────────────
#define CHAR_HOLD_TIME   2000   // How long to hold each letter (ms)
#define CHAR_GAP_TIME    500    // Pause between letters (ms)
#define SERVO_MOVE_TIME  300    // Time for servo to reach position (ms)

// ══════════════════════════════════════════════════════════════
// SERVO ANGLE LOOKUP TABLE — 8 cam positions per rack
// ══════════════════════════════════════════════════════════════
//
// HOME = 90° (rack centered, no dots raised)
//
// The servo rotates from 90° outward to slide the rack through
// 8 stepped cam positions. Each step is ~11° apart.
//
// The spur gear (30 teeth) converts servo rotation → linear
// rack motion. Each 11° of rotation moves the rack by one
// cam step on the 3D-printed profile.
//
//  Index  Binary  Dots Raised    Servo Angle    Offset from Home
//  ─────  ──────  ────────────   ───────────    ────────────────
//    0     000     None           90°            0° (HOME)
//    1     001     Dot 1 only     101°           +11°
//    2     010     Dot 2 only     112°           +22°
//    3     011     Dots 1,2       123°           +33°
//    4     100     Dot 3 only     134°           +44°
//    5     101     Dots 1,3       145°           +55°
//    6     110     Dots 2,3       156°           +66°
//    7     111     All three      167°           +77°
//
// ⚠️  CALIBRATION: After assembling your physical mechanism,
//     you may need to adjust these angles to match your
//     actual cam positions. Start with these values and
//     fine-tune by testing each position individually.
//
//     Use the 'pos' command (e.g., type "pos 3") to test
//     individual positions during calibration.
//
const int CAM_ANGLES[8] = {
   90,     // 000 → no dots raised (HOME — rack centered)
  101,     // 001 → dot 1 only
  112,     // 010 → dot 2 only
  123,     // 011 → dots 1, 2
  134,     // 100 → dot 3 only
  145,     // 101 → dots 1, 3
  156,     // 110 → dots 2, 3
  167      // 111 → all three dots
};

// ══════════════════════════════════════════════════════════════
// ASCII → BRAILLE 6-BIT PATTERN LOOKUP
// ══════════════════════════════════════════════════════════════
//
// Standard Braille encoding for letters a-z.
// Bit layout: [dot6][dot5][dot4][dot3][dot2][dot1]
//
// Braille cell:
//   [Dot 1] [Dot 4]    bit0  bit3
//   [Dot 2] [Dot 5]    bit1  bit4
//   [Dot 3] [Dot 6]    bit2  bit5
//
// Example: 'h' = dots 1,2,5 = bit0 + bit1 + bit4 = 0b010011
//

uint8_t charToBraille(char c) {
  // Convert to lowercase
  c = tolower(c);

  // Braille patterns for a-z (standard Grade 1 Braille)
  // Each value is the 6-bit pattern: [d6 d5 d4 | d3 d2 d1]
  switch (c) {
    case 'a': return 0b000001;  // dot 1
    case 'b': return 0b000011;  // dots 1,2
    case 'c': return 0b001001;  // dots 1,4
    case 'd': return 0b011001;  // dots 1,4,5
    case 'e': return 0b010001;  // dots 1,5
    case 'f': return 0b001011;  // dots 1,2,4
    case 'g': return 0b011011;  // dots 1,2,4,5
    case 'h': return 0b010011;  // dots 1,2,5
    case 'i': return 0b001010;  // dots 2,4
    case 'j': return 0b011010;  // dots 2,4,5
    case 'k': return 0b000101;  // dots 1,3
    case 'l': return 0b000111;  // dots 1,2,3
    case 'm': return 0b001101;  // dots 1,3,4
    case 'n': return 0b011101;  // dots 1,3,4,5
    case 'o': return 0b010101;  // dots 1,3,5
    case 'p': return 0b001111;  // dots 1,2,3,4
    case 'q': return 0b011111;  // dots 1,2,3,4,5
    case 'r': return 0b010111;  // dots 1,2,3,5
    case 's': return 0b001110;  // dots 2,3,4
    case 't': return 0b011110;  // dots 2,3,4,5
    case 'u': return 0b100101;  // dots 1,3,6
    case 'v': return 0b100111;  // dots 1,2,3,6
    case 'w': return 0b111010;  // dots 2,4,5,6
    case 'x': return 0b101101;  // dots 1,3,4,6
    case 'y': return 0b111101;  // dots 1,3,4,5,6
    case 'z': return 0b110101;  // dots 1,3,5,6
    case ' ': return 0b000000;  // space = no dots
    default:  return 0xFF;      // unsupported character
  }
}

// ── Servo Objects ──────────────────────────────────────────
Servo leftServo;
Servo rightServo;

// ── Serial Input Buffer ────────────────────────────────────
String inputBuffer = "";

// ══════════════════════════════════════════════════════════════
// SETUP — runs once on power-up / reset
// ══════════════════════════════════════════════════════════════
void setup() {
  // Start serial communication
  Serial.begin(115200);
  delay(1000);  // Give serial monitor time to connect

  Serial.println();
  Serial.println("══════════════════════════════════════════════");
  Serial.println("  BRAILLE MODULE v2 — Serial Monitor Test");
  Serial.println("  Servos: 2× MG90S | Home: 90° (centered)");
  Serial.println("══════════════════════════════════════════════");
  Serial.println();

  // Allocate ESP32 hardware timers for servo PWM
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);

  // Configure servo PWM frequency (standard 50 Hz for hobby servos)
  leftServo.setPeriodHertz(50);
  rightServo.setPeriodHertz(50);

  // Attach servos with MG90S pulse width range
  leftServo.attach(LEFT_SERVO_PIN, SERVO_MIN_US, SERVO_MAX_US);
  rightServo.attach(RIGHT_SERVO_PIN, SERVO_MIN_US, SERVO_MAX_US);

  Serial.print("[Servo] Left  MG90S attached on GPIO ");
  Serial.println(LEFT_SERVO_PIN);
  Serial.print("[Servo] Right MG90S attached on GPIO ");
  Serial.println(RIGHT_SERVO_PIN);

  // ── INITIALIZE TO 90° (HOME / CENTER POSITION) ───────────
  // The rack-and-pinion is designed so that at 90°, the rack
  // is in its starting/center position — no dots are raised.
  // This is CRITICAL for the cam mechanism to be in sync.
  Serial.println("[Init] Moving both servos to 90° (HOME — rack centered)...");
  leftServo.write(HOME_ANGLE);
  rightServo.write(HOME_ANGLE);
  delay(1000);  // Wait for servos to physically reach 90°
  Serial.println("[Init] ✓ Both servos at 90° — rack centered, no dots raised.");

  Serial.println();
  Serial.println("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
  Serial.println("  READY! Type a word in Serial Monitor and");
  Serial.println("  press Enter to display it in Braille.");
  Serial.println("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
  Serial.println();
  Serial.println("Commands:");
  Serial.println("  • Type any word/sentence → displays in Braille");
  Serial.println("  • Type 'test'   → runs all 26 letters a-z");
  Serial.println("  • Type 'home'   → reset servos to 90° (center)");
  Serial.println("  • Type 'sweep'  → sweep both servos 0°→180°→0°");
  Serial.println("  • Type 'pos N'  → move left servo to cam position N (0-7)");
  Serial.println("  • Type 'angle N'→ move left servo to exact angle N°");
  Serial.println("  • Type 'steps'  → step through all 8 cam positions");
  Serial.println();
}

// ══════════════════════════════════════════════════════════════
// LOOP — runs continuously
// ══════════════════════════════════════════════════════════════
void loop() {
  // Check if data is available on Serial
  if (Serial.available() > 0) {
    // Read the entire line (until newline)
    inputBuffer = Serial.readStringUntil('\n');
    inputBuffer.trim();  // Remove whitespace / carriage return

    if (inputBuffer.length() > 0) {
      Serial.println();
      Serial.print("► Received: \"");
      Serial.print(inputBuffer);
      Serial.println("\"");
      Serial.println();

      // ── Special Commands ──────────────────────────────────
      if (inputBuffer.equalsIgnoreCase("test")) {
        runTestAllLetters();
      }
      else if (inputBuffer.equalsIgnoreCase("home")) {
        homeServos();
        Serial.println("[Home] ✓ Both servos at 90° (centered).");
      }
      else if (inputBuffer.equalsIgnoreCase("sweep")) {
        runSweepTest();
      }
      else if (inputBuffer.equalsIgnoreCase("steps")) {
        runStepTest();
      }
      else if (inputBuffer.startsWith("pos ") || inputBuffer.startsWith("pos\t")) {
        // "pos N" → move left servo to cam position N (0-7)
        int pos = inputBuffer.substring(4).toInt();
        testCamPosition(pos);
      }
      else if (inputBuffer.startsWith("angle ") || inputBuffer.startsWith("angle\t")) {
        // "angle N" → move left servo to exact angle N
        int angle = inputBuffer.substring(6).toInt();
        testExactAngle(angle);
      }
      else {
        // ── Normal word/sentence input ──────────────────────
        displayWord(inputBuffer);
      }

      Serial.println();
      Serial.println("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
      Serial.println("  Ready for next input...");
      Serial.println("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
      Serial.println();
    }
  }
}

// ══════════════════════════════════════════════════════════════
// DISPLAY A WORD — letter by letter in Braille
// ══════════════════════════════════════════════════════════════
void displayWord(String word) {
  Serial.print("[Braille] Displaying ");
  Serial.print(word.length());
  Serial.println(" characters:");
  Serial.println();

  for (int i = 0; i < (int)word.length(); i++) {
    char c = word.charAt(i);

    // Get the 6-bit Braille pattern for this character
    uint8_t pattern = charToBraille(c);

    if (pattern == 0xFF) {
      // Unsupported character — skip
      Serial.print("  [");
      Serial.print(i + 1);
      Serial.print("] '");
      Serial.print(c);
      Serial.println("' → (unsupported, skipping)");
      continue;
    }

    // Display the character info
    Serial.print("  [");
    Serial.print(i + 1);
    Serial.print("] '");
    Serial.print(c);
    Serial.print("' → ");
    printBrailleInfo(c, pattern);

    // Drive the servos to display this pattern
    displayBraillePattern(pattern);

    // Hold the character so user can feel/read it
    delay(CHAR_HOLD_TIME);

    // Return servos to 90° (home) before next character
    homeServos();

    // Brief gap between characters
    if (i < (int)word.length() - 1) {
      delay(CHAR_GAP_TIME);
    }
  }

  Serial.println();
  Serial.println("[Braille] ✓ Word complete!");
}

// ══════════════════════════════════════════════════════════════
// DISPLAY A SINGLE BRAILLE PATTERN — drive both servos
// ══════════════════════════════════════════════════════════════
void displayBraillePattern(uint8_t pattern) {
  // Split 6-bit pattern into two 3-bit column values
  // Left column  = bits 0,1,2 → dots 1, 2, 3
  // Right column = bits 3,4,5 → dots 4, 5, 6
  uint8_t leftPattern  = pattern & 0x07;         // bits 0-2
  uint8_t rightPattern = (pattern >> 3) & 0x07;  // bits 3-5

  // Look up cam angle for each column
  int leftAngle  = CAM_ANGLES[leftPattern];
  int rightAngle = CAM_ANGLES[rightPattern];

  // Drive servos to position
  leftServo.write(leftAngle);
  rightServo.write(rightAngle);

  // Wait for servos to physically reach their target
  delay(SERVO_MOVE_TIME);

  // Debug: show angles
  Serial.print("         → Left servo: ");
  Serial.print(leftAngle);
  Serial.print("° (pos ");
  Serial.print(leftPattern);
  Serial.print(")  |  Right servo: ");
  Serial.print(rightAngle);
  Serial.print("° (pos ");
  Serial.print(rightPattern);
  Serial.println(")");
}

// ══════════════════════════════════════════════════════════════
// HOME SERVOS — return both to 90° (center / starting position)
// ══════════════════════════════════════════════════════════════
void homeServos() {
  leftServo.write(HOME_ANGLE);
  rightServo.write(HOME_ANGLE);
  delay(SERVO_MOVE_TIME);  // Wait for servos to reach 90°
}

// ══════════════════════════════════════════════════════════════
// PRINT BRAILLE CHARACTER INFO — visual dot layout
// ══════════════════════════════════════════════════════════════
void printBrailleInfo(char c, uint8_t pattern) {
  // Print binary pattern
  Serial.print("0b");
  for (int b = 5; b >= 0; b--) {
    Serial.print((pattern >> b) & 1);
  }

  // Print which dots are raised
  Serial.print("  Dots: ");
  bool first = true;
  for (int d = 0; d < 6; d++) {
    if (pattern & (1 << d)) {
      if (!first) Serial.print(",");
      Serial.print(d + 1);
      first = false;
    }
  }
  if (first) Serial.print("(none)");

  // Print visual Braille cell
  // Layout: [d1 d4] [d2 d5] [d3 d6]
  Serial.print("  Cell: ");
  Serial.print((pattern & 0x01) ? "●" : "○");
  Serial.print((pattern & 0x08) ? "●" : "○");
  Serial.print(" ");
  Serial.print((pattern & 0x02) ? "●" : "○");
  Serial.print((pattern & 0x10) ? "●" : "○");
  Serial.print(" ");
  Serial.print((pattern & 0x04) ? "●" : "○");
  Serial.println((pattern & 0x20) ? "●" : "○");
}

// ══════════════════════════════════════════════════════════════
// TEST INDIVIDUAL CAM POSITION — for calibration
// ══════════════════════════════════════════════════════════════
void testCamPosition(int pos) {
  if (pos < 0 || pos > 7) {
    Serial.println("[Error] Position must be 0-7!");
    return;
  }

  int angle = CAM_ANGLES[pos];
  Serial.print("[Calibrate] Moving LEFT servo to cam position ");
  Serial.print(pos);
  Serial.print(" (");

  // Show which dots this position should raise
  Serial.print("dots: ");
  if (pos == 0) Serial.print("none");
  if (pos & 0x01) Serial.print("1 ");
  if (pos & 0x02) Serial.print("2 ");
  if (pos & 0x04) Serial.print("3 ");

  Serial.print(") → angle: ");
  Serial.print(angle);
  Serial.println("°");

  leftServo.write(angle);
  delay(SERVO_MOVE_TIME);

  Serial.println("[Calibrate] Servo holding position. Type 'home' to return to 90°.");
  Serial.print("[Calibrate] If wrong, adjust CAM_ANGLES[");
  Serial.print(pos);
  Serial.println("] in the code.");
}

// ══════════════════════════════════════════════════════════════
// TEST EXACT ANGLE — for fine-tuning calibration
// ══════════════════════════════════════════════════════════════
void testExactAngle(int angle) {
  if (angle < 0 || angle > 180) {
    Serial.println("[Error] Angle must be 0-180!");
    return;
  }

  Serial.print("[Angle] Moving LEFT servo to exactly ");
  Serial.print(angle);
  Serial.println("°");

  leftServo.write(angle);
  delay(SERVO_MOVE_TIME);

  Serial.println("[Angle] Servo holding. Type 'home' to return to 90°.");
}

// ══════════════════════════════════════════════════════════════
// STEP TEST — walk through all 8 cam positions sequentially
// ══════════════════════════════════════════════════════════════
void runStepTest() {
  Serial.println("[Steps] Walking through all 8 cam positions on LEFT servo...");
  Serial.println("[Steps] Starting from HOME (90°)...");
  Serial.println();

  for (int pos = 0; pos < 8; pos++) {
    int angle = CAM_ANGLES[pos];

    Serial.print("  Position ");
    Serial.print(pos);
    Serial.print(" (");

    // Binary
    Serial.print((pos >> 2) & 1);
    Serial.print((pos >> 1) & 1);
    Serial.print(pos & 1);

    Serial.print(") → ");
    Serial.print(angle);
    Serial.print("° → Dots: ");

    if (pos == 0) Serial.print("none");
    if (pos & 0x01) Serial.print("1 ");
    if (pos & 0x02) Serial.print("2 ");
    if (pos & 0x04) Serial.print("3 ");

    Serial.println();

    leftServo.write(angle);
    delay(1500);  // Hold each position for 1.5s
  }

  // Return to home
  Serial.println();
  Serial.println("[Steps] Returning to HOME (90°)...");
  homeServos();
  Serial.println("[Steps] ✓ Step test complete!");
}

// ══════════════════════════════════════════════════════════════
// TEST ALL LETTERS a-z
// ══════════════════════════════════════════════════════════════
void runTestAllLetters() {
  Serial.println("[Test] Running through all 26 letters...");
  Serial.println("[Test] Home position: 90° (rack centered)");
  Serial.println();

  for (char c = 'a'; c <= 'z'; c++) {
    uint8_t pattern = charToBraille(c);

    Serial.print("  ");
    Serial.print(c);
    Serial.print(": ");
    printBrailleInfo(c, pattern);

    displayBraillePattern(pattern);
    delay(CHAR_HOLD_TIME);
    homeServos();
    delay(CHAR_GAP_TIME);
  }

  Serial.println();
  Serial.println("[Test] ✓ All 26 letters complete!");
}

// ══════════════════════════════════════════════════════════════
// SWEEP TEST — move servos through full range to verify wiring
// ══════════════════════════════════════════════════════════════
void runSweepTest() {
  Serial.println("[Sweep] Testing servo range 0° → 180° → 90° (home)");
  Serial.println("[Sweep] Left servo first, then right servo...");
  Serial.println();

  // Sweep left servo
  Serial.println("  Left MG90S: 90° → 180°");
  for (int angle = 90; angle <= 180; angle += 10) {
    leftServo.write(angle);
    Serial.print("    ");
    Serial.print(angle);
    Serial.println("°");
    delay(200);
  }
  Serial.println("  Left MG90S: 180° → 0°");
  for (int angle = 180; angle >= 0; angle -= 10) {
    leftServo.write(angle);
    Serial.print("    ");
    Serial.print(angle);
    Serial.println("°");
    delay(200);
  }
  Serial.println("  Left MG90S: 0° → 90° (home)");
  for (int angle = 0; angle <= 90; angle += 10) {
    leftServo.write(angle);
    delay(200);
  }
  Serial.println("  ✓ Left servo sweep done.");
  Serial.println();

  // Sweep right servo
  Serial.println("  Right MG90S: 90° → 180°");
  for (int angle = 90; angle <= 180; angle += 10) {
    rightServo.write(angle);
    Serial.print("    ");
    Serial.print(angle);
    Serial.println("°");
    delay(200);
  }
  Serial.println("  Right MG90S: 180° → 0°");
  for (int angle = 180; angle >= 0; angle -= 10) {
    rightServo.write(angle);
    Serial.print("    ");
    Serial.print(angle);
    Serial.println("°");
    delay(200);
  }
  Serial.println("  Right MG90S: 0° → 90° (home)");
  for (int angle = 0; angle <= 90; angle += 10) {
    rightServo.write(angle);
    delay(200);
  }
  Serial.println("  ✓ Right servo sweep done.");

  // Return to home
  homeServos();
  Serial.println();
  Serial.println("[Sweep] ✓ Sweep test complete. Both servos at 90° (home).");
}
