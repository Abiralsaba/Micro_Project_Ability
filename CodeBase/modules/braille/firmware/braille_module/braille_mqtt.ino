#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <ESP32Servo.h>

// WiFi — connects to Pi's Access Point
const char* WIFI_SSID = "Ability";
const char* WIFI_PASS = "ability123";

// MQTT — Pi is the broker
const char* MQTT_SERVER = "192.168.4.1";
const int   MQTT_PORT   = 1883;
const char* MODULE_ID   = "braille_01";

// MQTT Topics
const char* TOPIC_DISPLAY = "ability/braille/display";
const char* TOPIC_STATUS  = "ability/braille/status";
const char* TOPIC_HUB_CMD = "ability/hub/command";

// Servo pins
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

const int CAM_ANGLES[8] = { 32, 66, 180, 88, 154, 152, 132, 105 };
const int RIGHT_CAM_ANGLES[8] = { 22, 44, 170, 66, 148, 152, 132, 105 };

uint8_t charToBraille(char c) {
  c = tolower(c);
  switch (c) {
  case 'a': return 0b000001; case 'b': return 0b000011;
  case 'c': return 0b001001; case 'd': return 0b011001;
  case 'e': return 0b010001; case 'f': return 0b001011;
  case 'g': return 0b011011; case 'h': return 0b010011;
  case 'i': return 0b001010; case 'j': return 0b011010;
  case 'k': return 0b000101; case 'l': return 0b000111;
  case 'm': return 0b001101; case 'n': return 0b011101;
  case 'o': return 0b010101; case 'p': return 0b001111;
  case 'q': return 0b011111; case 'r': return 0b010111;
  case 's': return 0b001110; case 't': return 0b011110;
  case 'u': return 0b100101; case 'v': return 0b100111;
  case 'w': return 0b111010; case 'x': return 0b101101;
  case 'y': return 0b111101; case 'z': return 0b110101;
  case ' ': return 0b000000;
  default:  return 0xFF;
  }
}

Servo m1Left, m1Right;
Servo m2Left, m2Right;
Servo m3Left, m3Right;
Servo m4Left, m4Right;

bool moduleOK[NUM_MODULES] = { false, false, false, false };

WiFiClient wifiClient;
PubSubClient mqtt(wifiClient);

bool isBusy = false;
unsigned long lastStatusTime = 0;
#define STATUS_INTERVAL 10000

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
  Serial.println(rightAngle);
}

void homeModule(int module) {
  writeLeftServo(module, 22);
  delay(SERVO_SETTLE_MS);
  writeRightServo(module, 22);
  delay(SERVO_SETTLE_MS);
}

void homeAllServos() {
  for (int m = 0; m < NUM_MODULES; m++) {
    if (moduleOK[m]) homeModule(m);
  }
  delay(SERVO_MOVE_TIME);
}

void displayWord(String word) {
  isBusy = true;
  int totalChars = word.length();
  int totalBatches = (totalChars + NUM_MODULES - 1) / NUM_MODULES;

  Serial.print("Displaying: ");
  Serial.print(word);
  Serial.print(" (");
  Serial.print(totalBatches);
  Serial.println(" batches)");

  for (int batch = 0; batch < totalBatches; batch++) {
    int startIdx = batch * NUM_MODULES;
    int charsInBatch = min(NUM_MODULES, totalChars - startIdx);

    uint8_t patterns[NUM_MODULES];
    bool active[NUM_MODULES];

    for (int m = 0; m < NUM_MODULES; m++) {
      if (m < charsInBatch) {
        char c = word.charAt(startIdx + m);
        patterns[m] = charToBraille(c);
        active[m] = (patterns[m] != 0xFF) && moduleOK[m];
      } else {
        patterns[m] = 0b000000;
        active[m] = moduleOK[m];
      }
    }

    for (int m = 0; m < NUM_MODULES; m++) {
      if (active[m]) driveModule(m, patterns[m]);
    }

    delay(SERVO_MOVE_TIME);
    delay(CHAR_HOLD_TIME);
    homeAllServos();

    if (batch < totalBatches - 1) {
      delay(BATCH_GAP_TIME);
    }
  }

  Serial.println("Display complete.");
  isBusy = false;
}

// MQTT callback — called when a message arrives
void mqttCallback(char* topic, byte* payload, unsigned int length) {
  char msg[512];
  int len = min((unsigned int)511, length);
  memcpy(msg, payload, len);
  msg[len] = '\0';

  Serial.print("MQTT [");
  Serial.print(topic);
  Serial.print("]: ");
  Serial.println(msg);

  if (String(topic) == TOPIC_DISPLAY) {
    // Parse JSON
    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, msg);

    String text = "";
    if (err) {
      // Plain text, not JSON
      text = String(msg);
    } else {
      text = doc["text"].as<String>();
    }

    text.trim();
    if (text.length() == 0) return;

    if (isBusy) {
      Serial.println("Busy, ignoring.");
      return;
    }

    // Handle special commands
    if (text.equalsIgnoreCase("home")) {
      homeAllServos();
      Serial.println("Homed.");
    } else if (text.equalsIgnoreCase("test")) {
      runTestAllLetters();
    } else {
      displayWord(text);
    }
  }
}

void connectWiFi() {
  Serial.print("WiFi: Connecting to ");
  Serial.print(WIFI_SSID);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 30) {
    delay(500);
    Serial.print(".");
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println();
    Serial.print("WiFi connected! IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println();
    Serial.println("WiFi FAILED! Check SSID/password.");
  }
}

void connectMQTT() {
  while (!mqtt.connected()) {
    Serial.print("MQTT: Connecting to ");
    Serial.print(MQTT_SERVER);
    Serial.print("...");

    if (mqtt.connect(MODULE_ID)) {
      Serial.println(" connected!");
      mqtt.subscribe(TOPIC_DISPLAY);
      Serial.print("  Subscribed: ");
      Serial.println(TOPIC_DISPLAY);

      // Announce online
      publishStatus();
    } else {
      Serial.print(" failed (rc=");
      Serial.print(mqtt.state());
      Serial.println("). Retry in 3s...");
      delay(3000);
    }
  }
}

void publishStatus() {
  JsonDocument doc;
  doc["module"] = MODULE_ID;
  doc["online"] = true;
  doc["busy"] = isBusy;

  JsonArray mods = doc["modules_ok"].to<JsonArray>();
  for (int m = 0; m < NUM_MODULES; m++) {
    mods.add(moduleOK[m]);
  }

  doc["ip"] = WiFi.localIP().toString();
  doc["rssi"] = WiFi.RSSI();

  char buffer[256];
  serializeJson(doc, buffer);
  mqtt.publish(TOPIC_STATUS, buffer);
}

void runTestAllLetters() {
  isBusy = true;
  Serial.println("Running a-z test...");

  for (int i = 0; i < 26; i += NUM_MODULES) {
    int charsInBatch = min(NUM_MODULES, 26 - i);

    for (int m = 0; m < charsInBatch; m++) {
      char c = 'a' + i + m;
      uint8_t pattern = charToBraille(c);
      if (moduleOK[m]) driveModule(m, pattern);
    }

    delay(SERVO_MOVE_TIME);
    delay(CHAR_HOLD_TIME);
    homeAllServos();
    delay(BATCH_GAP_TIME);
  }

  Serial.println("Test complete!");
  isBusy = false;
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println();
  Serial.println("BRAILLE MODULE (MQTT)");
  Serial.println();

  // Servo setup
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);

  m1Left.setPeriodHertz(50);
  m1Left.attach(M1_LEFT_PIN, SERVO_MIN_US, SERVO_MAX_US); delay(100);
  m1Right.setPeriodHertz(50);
  m1Right.attach(M1_RIGHT_PIN, SERVO_MIN_US, SERVO_MAX_US); delay(100);
  moduleOK[0] = m1Left.attached() && m1Right.attached();
  Serial.println(moduleOK[0] ? "M1: OK" : "M1: FAILED");

  m2Left.setPeriodHertz(50);
  m2Left.attach(M2_LEFT_PIN, SERVO_MIN_US, SERVO_MAX_US); delay(100);
  m2Right.setPeriodHertz(50);
  m2Right.attach(M2_RIGHT_PIN, SERVO_MIN_US, SERVO_MAX_US); delay(100);
  moduleOK[1] = m2Left.attached() && m2Right.attached();
  Serial.println(moduleOK[1] ? "M2: OK" : "M2: FAILED");

  m3Left.setPeriodHertz(50);
  m3Left.attach(M3_LEFT_PIN, SERVO_MIN_US, SERVO_MAX_US); delay(100);
  m3Right.setPeriodHertz(50);
  m3Right.attach(M3_RIGHT_PIN, SERVO_MIN_US, SERVO_MAX_US); delay(100);
  moduleOK[2] = m3Left.attached() && m3Right.attached();
  Serial.println(moduleOK[2] ? "M3: OK" : "M3: FAILED");

  m4Left.setPeriodHertz(50);
  m4Left.attach(M4_LEFT_PIN, SERVO_MIN_US, SERVO_MAX_US); delay(100);
  m4Right.setPeriodHertz(50);
  m4Right.attach(M4_RIGHT_PIN, SERVO_MIN_US, SERVO_MAX_US); delay(100);
  moduleOK[3] = m4Left.attached() && m4Right.attached();
  Serial.println(moduleOK[3] ? "M4: OK" : "M4: FAILED");

  homeAllServos();
  delay(500);
  Serial.println("Servos at home.");

  // WiFi + MQTT
  connectWiFi();

  mqtt.setServer(MQTT_SERVER, MQTT_PORT);
  mqtt.setCallback(mqttCallback);
  mqtt.setBufferSize(512);

  connectMQTT();

  Serial.println();
  Serial.println("READY — waiting for MQTT messages...");
  Serial.println("Serial input also works.");
  Serial.println();
}

void loop() {
  // Maintain WiFi
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi lost. Reconnecting...");
    connectWiFi();
  }

  // Maintain MQTT
  if (!mqtt.connected()) {
    connectMQTT();
  }
  mqtt.loop();

  // Periodic status publish
  if (millis() - lastStatusTime > STATUS_INTERVAL) {
    publishStatus();
    lastStatusTime = millis();
  }

  // Serial input (for debugging without MQTT)
  if (Serial.available() > 0) {
    String input = Serial.readStringUntil('\n');
    input.trim();
    if (input.length() > 0 && !isBusy) {
      Serial.print("Serial: ");
      Serial.println(input);

      if (input.equalsIgnoreCase("home")) {
        homeAllServos();
      } else if (input.equalsIgnoreCase("test")) {
        runTestAllLetters();
      } else {
        displayWord(input);
      }
    }
  }
}
