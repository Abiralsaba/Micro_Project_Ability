#include <ArduinoJson.h>
#include <ESP32Servo.h>
#include <PubSubClient.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <time.h>

#include "emqx_ca.h"
#include "secrets.h"

// Cloud transport. The Raspberry Pi publishes device-ready six-dot patterns.
const char* MQTT_SERVER = "z91cfe11.ala.asia-southeast1.emqxsl.com";
const uint16_t MQTT_PORT = 8883;
const char* MODULE_ID = "braille_01";
const char* TOPIC_DISPLAY = "ability/v1/braille_01/output";
const char* TOPIC_STATUS = "ability/v1/braille_01/status";

// Exact pin map from the calibrated demomodule.ino.
#define M1_LEFT_PIN 13
#define M1_RIGHT_PIN 12
#define M2_LEFT_PIN 14
#define M2_RIGHT_PIN 27
#define M3_LEFT_PIN 26
#define M3_RIGHT_PIN 25
#define M4_LEFT_PIN 33
#define M4_RIGHT_PIN 32

#define NUM_MODULES 4
#define SERVO_MIN_US 500
#define SERVO_MAX_US 2400
#define MAX_PATTERNS 64

// Exact calibrated timing from demomodule.ino.
#define CHAR_HOLD_TIME 5000
#define BATCH_GAP_TIME 3000
#define SERVO_MOVE_TIME 300
#define SERVO_SETTLE_MS 30

// Exact calibrated positions from demomodule.ino.
const int CAM_ANGLES[8] = {
  32, 66, 180, 88, 154, 152, 132, 105
};

const int RIGHT_CAM_ANGLES[8] = {
  22, 44, 170, 66, 148, 152, 132, 105
};

Servo m1Left;
Servo m1Right;
Servo m2Left;
Servo m2Right;
Servo m3Left;
Servo m3Right;
Servo m4Left;
Servo m4Right;

bool moduleOK[NUM_MODULES] = {false, false, false, false};
WiFiClientSecure secureClient;
PubSubClient mqtt(secureClient);

uint8_t pendingPatterns[MAX_PATTERNS];
uint8_t pendingCount = 0;
char pendingMessageId[48] = "";
bool hasPending = false;
bool isBusy = false;
bool clockReady = false;

unsigned long lastStatusTime = 0;
unsigned long lastWifiAttempt = 0;
unsigned long lastMqttAttempt = 0;

const unsigned long STATUS_INTERVAL = 10000;
const unsigned long WIFI_RETRY_INTERVAL = 10000;
const unsigned long MQTT_RETRY_INTERVAL = 5000;

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
    default: return 0xFF;
  }
}

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

  // The right rack is physically mirrored. Keep this exact reversal.
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
  Serial.print(": pattern=");
  Serial.print(pattern);
  Serial.print(" L=");
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
  for (int module = 0; module < NUM_MODULES; module++) {
    if (moduleOK[module]) homeModule(module);
  }
  delay(SERVO_MOVE_TIME);
}

void serviceDelay(unsigned long durationMs) {
  unsigned long started = millis();
  while (millis() - started < durationMs) {
    if (mqtt.connected()) mqtt.loop();
    delay(10);
  }
}

void publishStatus(const char* eventName = "status", const char* messageId = "") {
  if (!mqtt.connected()) return;

  JsonDocument doc;
  doc["module"] = MODULE_ID;
  doc["online"] = true;
  doc["busy"] = isBusy;
  doc["queued"] = hasPending;
  doc["event"] = eventName;
  if (messageId[0] != '\0') doc["message_id"] = messageId;
  doc["rssi"] = WiFi.RSSI();

  JsonArray modules = doc["modules_ok"].to<JsonArray>();
  for (int module = 0; module < NUM_MODULES; module++) {
    modules.add(moduleOK[module]);
  }

  char buffer[384];
  size_t length = serializeJson(doc, buffer, sizeof(buffer));
  mqtt.publish(TOPIC_STATUS, reinterpret_cast<const uint8_t*>(buffer), length, false);
}

void displayPatterns(const uint8_t* patterns, int patternCount, const char* messageId) {
  isBusy = true;
  publishStatus("display_started", messageId);

  int totalBatches = (patternCount + NUM_MODULES - 1) / NUM_MODULES;
  Serial.print("Displaying ");
  Serial.print(patternCount);
  Serial.print(" patterns in ");
  Serial.print(totalBatches);
  Serial.println(" batch(es)");

  for (int batch = 0; batch < totalBatches; batch++) {
    int startIndex = batch * NUM_MODULES;
    int patternsInBatch = min(NUM_MODULES, patternCount - startIndex);

    Serial.print("Batch ");
    Serial.print(batch + 1);
    Serial.print("/");
    Serial.println(totalBatches);

    for (int module = 0; module < NUM_MODULES; module++) {
      uint8_t pattern = 0b000000;
      if (module < patternsInBatch) {
        pattern = patterns[startIndex + module] & 0x3F;
      }
      if (moduleOK[module]) driveModule(module, pattern);
    }

    serviceDelay(SERVO_MOVE_TIME);
    serviceDelay(CHAR_HOLD_TIME);
    homeAllServos();

    if (batch < totalBatches - 1) {
      serviceDelay(BATCH_GAP_TIME);
    }
  }

  isBusy = false;
  publishStatus("display_complete", messageId);
  Serial.println("Display complete.");
}

void displayDiagnosticText(String text) {
  uint8_t patterns[MAX_PATTERNS];
  uint8_t count = 0;

  for (unsigned int index = 0; index < text.length() && count < MAX_PATTERNS; index++) {
    uint8_t pattern = charToBraille(text.charAt(index));
    if (pattern != 0xFF) patterns[count++] = pattern;
  }

  if (count > 0) displayPatterns(patterns, count, "serial-diagnostic");
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  if (String(topic) != TOPIC_DISPLAY) return;

  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, payload, length);
  if (error) {
    Serial.print("Invalid MQTT JSON: ");
    Serial.println(error.c_str());
    return;
  }

  if (hasPending) {
    Serial.println("Pending slot full; message rejected.");
    const char* rejectedId = doc["message_id"] | "unknown";
    publishStatus("queue_rejected", rejectedId);
    return;
  }

  JsonArray patterns = doc["braille"].as<JsonArray>();
  if (patterns.isNull()) {
    Serial.println("MQTT message has no braille array.");
    return;
  }

  pendingCount = 0;
  for (JsonVariant value : patterns) {
    if (pendingCount >= MAX_PATTERNS) break;
    int pattern = value.as<int>();
    if (pattern >= 0 && pattern <= 63) {
      pendingPatterns[pendingCount++] = static_cast<uint8_t>(pattern);
    }
  }

  if (pendingCount == 0) {
    Serial.println("MQTT braille array is empty or invalid.");
    return;
  }

  const char* messageId = doc["message_id"] | "unknown";
  strlcpy(pendingMessageId, messageId, sizeof(pendingMessageId));
  hasPending = true;
  Serial.print("Queued calibrated Braille patterns: ");
  Serial.println(pendingCount);
  publishStatus("queued", pendingMessageId);
}

bool connectWiFi() {
  if (WiFi.status() == WL_CONNECTED) return true;

  Serial.print("Connecting to Wi-Fi: ");
  Serial.println(ABILITY_WIFI_SSID);
  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false);
  WiFi.setAutoReconnect(true);
  WiFi.begin(ABILITY_WIFI_SSID, ABILITY_WIFI_PASSWORD);

  for (int attempt = 0; attempt < 40; attempt++) {
    if (WiFi.status() == WL_CONNECTED) {
      Serial.print("Wi-Fi connected. IP: ");
      Serial.println(WiFi.localIP());
      return true;
    }
    delay(500);
    Serial.print('.');
  }

  Serial.println("\nWi-Fi connection timed out; will retry.");
  return false;
}

bool syncClock() {
  if (clockReady) return true;

  configTime(0, 0, "pool.ntp.org", "time.google.com");
  Serial.print("Synchronizing clock");
  for (int attempt = 0; attempt < 30; attempt++) {
    if (time(nullptr) >= 1700000000) {
      clockReady = true;
      Serial.println(" synchronized.");
      return true;
    }
    delay(500);
    Serial.print('.');
  }

  Serial.println("\nClock synchronization timed out; will retry.");
  return false;
}

void connectMQTT() {
  if (mqtt.connected() || WiFi.status() != WL_CONNECTED || !clockReady) return;
  if (millis() - lastMqttAttempt < MQTT_RETRY_INTERVAL) return;
  lastMqttAttempt = millis();

  String clientId = String(MODULE_ID) + "-" + String((uint32_t)ESP.getEfuseMac(), HEX);
  const char* offline = "{\"module\":\"braille_01\",\"online\":false}";

  Serial.print("Connecting securely to EMQX...");
  bool connected = mqtt.connect(
    clientId.c_str(),
    ABILITY_MQTT_USERNAME,
    ABILITY_MQTT_PASSWORD,
    TOPIC_STATUS,
    1,
    true,
    offline
  );

  if (connected) {
    Serial.println(" connected.");
    mqtt.subscribe(TOPIC_DISPLAY, 1);
    Serial.print("Subscribed: ");
    Serial.println(TOPIC_DISPLAY);
    publishStatus("connected");
  } else {
    Serial.print(" failed, MQTT state=");
    Serial.println(mqtt.state());
  }
}

void setupServosExactlyLikeDemo() {
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
  for (int module = 0; module < NUM_MODULES; module++) {
    if (moduleOK[module]) okCount++;
  }
  Serial.print(okCount);
  Serial.println("/4 modules attached.");

  Serial.println("Moving all servos to calibrated home (22)...");
  homeAllServos();
  delay(1000);
  Serial.println("All servos at home.");
}

void runDiagnostics() {
  if (isBusy) return;
  isBusy = true;

  for (int module = 0; module < NUM_MODULES; module++) {
    if (!moduleOK[module]) continue;
    writeLeftServo(module, 90);
    serviceDelay(500);
    writeLeftServo(module, 22);
    serviceDelay(500);
    writeRightServo(module, 90);
    serviceDelay(500);
    writeRightServo(module, 22);
    serviceDelay(500);
    driveModule(module, charToBraille('a'));
    serviceDelay(2000);
    homeModule(module);
    serviceDelay(300);
  }

  homeAllServos();
  isBusy = false;
}

void runSweepTest() {
  if (isBusy) return;
  isBusy = true;

  for (int module = 0; module < NUM_MODULES; module++) {
    if (!moduleOK[module]) continue;
    for (int angle = 0; angle <= 180; angle += 10) {
      writeLeftServo(module, angle);
      serviceDelay(150);
    }
    for (int angle = 180; angle >= 0; angle -= 10) {
      writeLeftServo(module, angle);
      serviceDelay(150);
    }
    for (int angle = 0; angle <= 180; angle += 10) {
      writeRightServo(module, angle);
      serviceDelay(150);
    }
    for (int angle = 180; angle >= 0; angle -= 10) {
      writeRightServo(module, angle);
      serviceDelay(150);
    }
  }

  homeAllServos();
  isBusy = false;
}

void handleSerialCommand() {
  if (Serial.available() <= 0 || isBusy) return;

  String command = Serial.readStringUntil('\n');
  command.trim();
  if (command.length() == 0) return;

  if (command.equalsIgnoreCase("home")) {
    homeAllServos();
  } else if (command.equalsIgnoreCase("test")) {
    displayDiagnosticText("abcdefghijklmnopqrstuvwxyz");
  } else if (command.equalsIgnoreCase("diag")) {
    runDiagnostics();
  } else if (command.equalsIgnoreCase("sweep")) {
    runSweepTest();
  } else {
    displayDiagnosticText(command);
  }
}

void setup() {
  Serial.begin(115200);

  // Allow a non-USB power supply and its voltage regulator to stabilize.
  delay(3000);
  Serial.println("\nBRAILLE 4-MODULE CLOUD DISPLAY");
  Serial.println("Calibrated actuator behavior: demomodule.ino");

  // Bring up networking before the eight servo channels create current spikes.
  lastWifiAttempt = millis();
  if (connectWiFi()) {
    syncClock();
  }

  secureClient.setCACert(EMQX_ROOT_CA);
  mqtt.setServer(MQTT_SERVER, MQTT_PORT);
  mqtt.setCallback(mqttCallback);
  mqtt.setBufferSize(2048);
  mqtt.setKeepAlive(30);

  setupServosExactlyLikeDemo();

  lastMqttAttempt = millis() - MQTT_RETRY_INTERVAL;
  connectMQTT();

  Serial.println("READY");
  Serial.println("Serial diagnostics: home, test, diag, sweep");
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    if (millis() - lastWifiAttempt >= WIFI_RETRY_INTERVAL) {
      lastWifiAttempt = millis();
      if (connectWiFi()) syncClock();
    }
  } else {
    if (!clockReady) syncClock();
    connectMQTT();
    mqtt.loop();
  }

  if (hasPending && !isBusy) {
    uint8_t currentPatterns[MAX_PATTERNS];
    uint8_t currentCount = pendingCount;
    char currentMessageId[48];

    memcpy(currentPatterns, pendingPatterns, currentCount);
    strlcpy(currentMessageId, pendingMessageId, sizeof(currentMessageId));
    hasPending = false;
    pendingCount = 0;

    displayPatterns(currentPatterns, currentCount, currentMessageId);
  }

  handleSerialCommand();

  if (millis() - lastStatusTime >= STATUS_INTERVAL) {
    publishStatus();
    lastStatusTime = millis();
  }

  delay(5);
}
