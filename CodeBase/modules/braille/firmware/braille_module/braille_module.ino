/*
 * ══════════════════════════════════════════════════════════════
 * BRAILLE MODULE FIRMWARE — Project Ability
 * Target: ESP32-S3 WROOM (Arduino IDE)
 * Board: "ESP32S3 Dev Module" in Arduino IDE
 * ══════════════════════════════════════════════════════════════
 *
 * PURPOSE:
 *   Reads Braille button input (6 keys, Perkins layout),
 *   decodes to TEXT, and sends to Central Hub via WiFi MQTT.
 *   Receives Braille byte encoding from Hub → splits into
 *   left (dots 1,2,3) and right (dots 4,5,6) columns →
 *   commands 2 servo motors → rack-and-pinion gears with
 *   stepped cam profiles selectively raise the correct pins.
 *
 * BRAILLE OUTPUT MECHANISM (2-Servo Rack & Pinion):
 *   LEFT SERVO  → Left Pinion → Left Rack (cam)  → Dots 1,2,3
 *   RIGHT SERVO → Right Pinion → Right Rack (cam) → Dots 4,5,6
 *
 *   Each rack has 8 cam positions (3 dots = 2³ = 8 combos).
 *   Left × Right = 8 × 8 = 64 = ALL Braille patterns.
 *
 *   Each position = ~22° of servo rotation (180° / 8).
 *   SG90 precision ~1° → easily handles 8 discrete steps.
 *
 * HARDWARE:
 *   - 6× Tactile push buttons (Braille input, Perkins layout)
 *   - 1× Spacebar button (word separator)
 *   - 2× Micro Servo Motors (SG90/MG90S) — driven directly by ESP32 PWM
 *   - 2× Rack Bars with stepped cam profile (3D printed)
 *   - 2× Pinion Gears (3D printed)
 *   - 6× Metallic Pin Rods (2mm diameter)
 *   - 3D Printed Pin Housing + Frame
 *   - MAX98357A I2S Amplifier + 3W Speaker (TTS audio)
 *   - NO PCA9685 needed (only 2 servos = 2 GPIO PWM pins)
 *
 * LIBRARIES REQUIRED (install via Arduino IDE Library Manager):
 *   - ESP32Servo (for servo control on ESP32)
 *   - ArduinoJson
 *   - PubSubClient (MQTT)
 *   - WiFi (built-in for ESP32)
 *
 * BOARD SETUP (Arduino IDE):
 *   1. Tools → Board → ESP32 Arduino → "ESP32S3 Dev Module"
 *   2. Tools → USB CDC On Boot → "Enabled"
 *   3. Tools → Upload Speed → 921600
 */

#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <ESP32Servo.h>

// ── WiFi & MQTT Configuration ───────────────────────────────
const char* WIFI_SSID     = "YOUR_WIFI_SSID";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";
const char* MQTT_SERVER   = "CENTRAL_HUB_IP";
const int   MQTT_PORT     = 1883;
const char* MODULE_ID     = "braille_user_01";

// ── Pin Definitions — Braille Buttons (INPUT) ───────────────
// Perkins layout: [Dot3] [Dot2] [Dot1]  |SPACE|  [Dot4] [Dot5] [Dot6]
#define BTN_DOT1  12
#define BTN_DOT2  13
#define BTN_DOT3  14
#define BTN_DOT4  15
#define BTN_DOT5  16
#define BTN_DOT6  17
#define BTN_SPACE 33
#define BTN_SEND  32   // Send message button

// ── Pin Definitions — 2 Servo Motors (PWM output) ───────────
#define LEFT_SERVO_PIN   25   // Controls left rack → Dots 1,2,3
#define RIGHT_SERVO_PIN  26   // Controls right rack → Dots 4,5,6

// ── Pin Definitions — I2S Speaker ───────────────────────────
#define I2S_BCLK  35
#define I2S_LRC   36
#define I2S_DOUT  37

// ══════════════════════════════════════════════════════════════
// SERVO ANGLE LOOKUP TABLE
// ══════════════════════════════════════════════════════════════
//
// Each rack has a 3D-printed stepped cam profile with 8 positions.
// Each position corresponds to a unique combination of 3 pin states.
// Index = 3-bit binary pattern (dots in that column).
//
// Map: 3-bit pattern → servo angle (degrees)
// Calibrate these values after assembling the physical mechanism!
//
// Index  Binary  Dots Raised     Servo Angle
// ─────  ──────  ──────────────  ───────────
//   0     000    None            0°
//   1     001    Dot 1 only      22°
//   2     010    Dot 2 only      44°
//   3     011    Dots 1,2        66°
//   4     100    Dot 3 only      88°
//   5     101    Dots 1,3        110°
//   6     110    Dots 2,3        132°
//   7     111    All three       154°
//
const int CAM_ANGLES[8] = {
  0,     // 000 → no dots
  22,    // 001 → dot 1
  44,    // 010 → dot 2
  66,    // 011 → dots 1,2
  88,    // 100 → dot 3
  110,   // 101 → dots 1,3
  132,   // 110 → dots 2,3
  154    // 111 → all three
};

// ── Braille Lookup Table (6-bit → ASCII) ────────────────────
// Bit order: [dot6][dot5][dot4][dot3][dot2][dot1]
const char BRAILLE_TO_ASCII[64] = {
  ' ',  'a', 'b', ',', 'k', ';', 'c', '-',  // 0x00-0x07
  'i',  's', 'f', '!', 'm', 'e', ':', 'h',  // 0x08-0x0F
  'd',  'j', 'g', '?', 'n', '.', 'o', '"',  // 0x10-0x17
  'p',  'r', 'l', '(', 't', ')', 'u', '\'', // 0x18-0x1F
  'q',  '1', 'v', '2', 'x', '3', '&', '4',  // 0x20-0x27
  'z',  '5', '/', '6', '-', '7', 'y', '8',  // 0x28-0x2F
  'w',  '9', '#', '0', ' ', ' ', ' ', ' ',  // 0x30-0x37
  ' ',  ' ', ' ', ' ', ' ', ' ', ' ', ' '   // 0x38-0x3F
};

// ── Global Objects ──────────────────────────────────────────
WiFiClient   wifiClient;
PubSubClient mqttClient(wifiClient);
Servo        leftServo;
Servo        rightServo;

String currentMessage = "";

void setup() {
  Serial.begin(115200);
  Serial.println("[Ability] Braille Module — Initializing...");
  Serial.println("[Info] 2-Servo Rack & Pinion mechanism");

  // Attach servos to GPIO pins
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  leftServo.setPeriodHertz(50);
  rightServo.setPeriodHertz(50);
  leftServo.attach(LEFT_SERVO_PIN, 500, 2400);
  rightServo.attach(RIGHT_SERVO_PIN, 500, 2400);
  Serial.println("[Servos] Left (GPIO 25) + Right (GPIO 26) attached");

  // Button pins (INPUT_PULLUP — active LOW)
  pinMode(BTN_DOT1, INPUT_PULLUP);
  pinMode(BTN_DOT2, INPUT_PULLUP);
  pinMode(BTN_DOT3, INPUT_PULLUP);
  pinMode(BTN_DOT4, INPUT_PULLUP);
  pinMode(BTN_DOT5, INPUT_PULLUP);
  pinMode(BTN_DOT6, INPUT_PULLUP);
  pinMode(BTN_SPACE, INPUT_PULLUP);
  pinMode(BTN_SEND, INPUT_PULLUP);

  // Retract all pins on startup (position 0)
  retractAllPins();

  // WiFi + MQTT
  connectWiFi();
  mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
  mqttClient.setCallback(mqttCallback);
  connectMQTT();

  Serial.println("[Ability] Braille Module — Ready!");
}

void loop() {
  if (!mqttClient.connected()) connectMQTT();
  mqttClient.loop();

  readBrailleInput();
}

// ══════════════════════════════════════════════════════════════
// BRAILLE INPUT — Reading buttons → decoding → building message
// ══════════════════════════════════════════════════════════════

void readBrailleInput() {
  // Check if SEND button pressed
  if (digitalRead(BTN_SEND) == LOW) {
    delay(200);  // Debounce
    if (currentMessage.length() > 0) {
      sendMessage(currentMessage);
      currentMessage = "";
    }
    return;
  }

  // Check if SPACE button pressed
  if (digitalRead(BTN_SPACE) == LOW) {
    delay(200);  // Debounce
    currentMessage += ' ';
    Serial.print("[Input] Space — Message so far: ");
    Serial.println(currentMessage);
    return;
  }

  // Read all 6 dot buttons simultaneously
  uint8_t pattern = 0;
  if (digitalRead(BTN_DOT1) == LOW) pattern |= 0x01;
  if (digitalRead(BTN_DOT2) == LOW) pattern |= 0x02;
  if (digitalRead(BTN_DOT3) == LOW) pattern |= 0x04;
  if (digitalRead(BTN_DOT4) == LOW) pattern |= 0x08;
  if (digitalRead(BTN_DOT5) == LOW) pattern |= 0x10;
  if (digitalRead(BTN_DOT6) == LOW) pattern |= 0x20;

  if (pattern > 0) {
    delay(300);  // Wait for user to press all dots for this character
    char decoded = BRAILLE_TO_ASCII[pattern];
    currentMessage += decoded;
    Serial.print("[Input] Braille 0x");
    Serial.print(pattern, HEX);
    Serial.print(" → '");
    Serial.print(decoded);
    Serial.print("' — Message: ");
    Serial.println(currentMessage);
  }
}

// ══════════════════════════════════════════════════════════════
// BRAILLE OUTPUT — 2-Servo Rack & Pinion Cam Actuation
// ══════════════════════════════════════════════════════════════

/*
 * displayBrailleCharacter()
 *
 * Takes a 6-bit Braille pattern and splits it into two 3-bit columns:
 *   Left column  (bits 0-2) = dots 1, 2, 3 → left servo position
 *   Right column (bits 3-5) = dots 4, 5, 6 → right servo position
 *
 * Each servo drives a pinion gear → rack bar slides to one of 8
 * positions → the stepped cam profile on the rack selectively
 * raises the correct pin rods through the housing.
 *
 * Braille cell layout:
 *   [Dot 1] [Dot 4]     Left: bits 0,1,2    Right: bits 3,4,5
 *   [Dot 2] [Dot 5]
 *   [Dot 3] [Dot 6]
 */
void displayBrailleCharacter(uint8_t pattern) {
  // Split 6-bit pattern into two 3-bit column values
  uint8_t leftPattern  = pattern & 0x07;          // bits 0-2 → dots 1,2,3
  uint8_t rightPattern = (pattern >> 3) & 0x07;   // bits 3-5 → dots 4,5,6

  // Look up the cam angle for each column
  int leftAngle  = CAM_ANGLES[leftPattern];
  int rightAngle = CAM_ANGLES[rightPattern];

  // Command servos → pinion rotates → rack slides → cam raises pins
  leftServo.write(leftAngle);
  rightServo.write(rightAngle);

  // Debug output
  Serial.print("[Output] Pattern 0x");
  Serial.print(pattern, HEX);
  Serial.print(" → L:");
  Serial.print(leftPattern, BIN);
  Serial.print("(");
  Serial.print(leftAngle);
  Serial.print("°) R:");
  Serial.print(rightPattern, BIN);
  Serial.print("(");
  Serial.print(rightAngle);
  Serial.print("°) Pins: ");

  // Visual pin state
  uint8_t bits[6] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20};
  for (int i = 0; i < 6; i++) {
    Serial.print((pattern & bits[i]) ? "▲" : "▽");
    if (i == 2) Serial.print("|");
  }
  Serial.println();
}

/*
 * retractAllPins()
 * Both servos to position 0 (0°) → both racks fully retracted
 * → all 6 pins flush with surface.
 */
void retractAllPins() {
  leftServo.write(CAM_ANGLES[0]);    // 0° → no dots
  rightServo.write(CAM_ANGLES[0]);   // 0° → no dots
  Serial.println("[Servos] All pins retracted (position 0)");
}

// ══════════════════════════════════════════════════════════════
// MQTT — Sending messages & receiving Braille output
// ══════════════════════════════════════════════════════════════

void sendMessage(String text) {
  JsonDocument doc;
  doc["module_id"] = MODULE_ID;
  doc["profile"]   = "blind";
  doc["text"]      = text;

  char buffer[512];
  serializeJson(doc, buffer);

  String topic = "ability/local/" + String(MODULE_ID) + "/input";
  mqttClient.publish(topic.c_str(), buffer);

  Serial.print("[MQTT] Sent: ");
  Serial.println(text);
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  // Parse incoming Braille data from Central Hub
  JsonDocument doc;
  deserializeJson(doc, payload, length);

  if (doc.containsKey("braille")) {
    JsonArray brailleBytes = doc["braille"].as<JsonArray>();
    Serial.print("[Output] Displaying Braille message (");
    Serial.print(brailleBytes.size());
    Serial.println(" characters):");

    for (int i = 0; i < brailleBytes.size(); i++) {
      uint8_t pattern = brailleBytes[i];
      displayBrailleCharacter(pattern);
      delay(1500);         // Hold character for 1.5 seconds (user reads)
      retractAllPins();     // Retract all pins
      delay(300);           // Brief pause before next character
    }
    Serial.println("[Output] Message complete.");
  }
}

void connectWiFi() {
  Serial.print("[WiFi] Connecting...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" Connected! IP: " + WiFi.localIP().toString());
}

void connectMQTT() {
  while (!mqttClient.connected()) {
    Serial.print("[MQTT] Connecting...");
    if (mqttClient.connect(MODULE_ID)) {
      Serial.println(" Connected!");
      String topic = "ability/local/" + String(MODULE_ID) + "/output";
      mqttClient.subscribe(topic.c_str());
    } else {
      Serial.print(" Failed. Retrying...");
      delay(5000);
    }
  }
}
