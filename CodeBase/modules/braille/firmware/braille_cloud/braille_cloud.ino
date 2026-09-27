#include <ArduinoJson.h>
#include <ESP32Servo.h>
#include <PubSubClient.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <time.h>

#include "emqx_ca.h"
#include "secrets.h"

const char* MQTT_SERVER = "z91cfe11.ala.asia-southeast1.emqxsl.com";
const uint16_t MQTT_PORT = 8883;
const char* MODULE_ID = "braille_01";
const char* TOPIC_DISPLAY = "ability/v1/braille_01/output";
const char* TOPIC_STATUS = "ability/v1/braille_01/status";

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
#define QUEUE_SIZE 4

const unsigned long CHAR_HOLD_TIME = 5000;
const unsigned long BATCH_GAP_TIME = 3000;
const unsigned long STATUS_INTERVAL = 10000;
const unsigned long RECONNECT_INTERVAL = 5000;
const int SERVO_SETTLE_MS = 30;

const int CAM_ANGLES[8] = {32, 66, 180, 88, 154, 152, 132, 105};
const int RIGHT_CAM_ANGLES[8] = {22, 44, 170, 66, 148, 152, 132, 105};

Servo m1Left, m1Right;
Servo m2Left, m2Right;
Servo m3Left, m3Right;
Servo m4Left, m4Right;
bool moduleOK[NUM_MODULES] = {false, false, false, false};

WiFiClientSecure secureClient;
PubSubClient mqtt(secureClient);

struct BrailleMessage {
  uint8_t patterns[MAX_PATTERNS];
  uint8_t length;
  char messageId[48];
};

BrailleMessage messageQueue[QUEUE_SIZE];
uint8_t queueHead = 0;
uint8_t queueTail = 0;
uint8_t queueCount = 0;
BrailleMessage currentMessage;
uint8_t currentOffset = 0;

enum DisplayState { DISPLAY_IDLE, DISPLAY_HOLDING, DISPLAY_GAP };
DisplayState displayState = DISPLAY_IDLE;
unsigned long stateDeadline = 0;
unsigned long lastStatusTime = 0;
unsigned long lastMqttAttempt = 0;

bool isBusy() {
  return displayState != DISPLAY_IDLE || queueCount > 0;
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
  pattern &= 0x3F;
  uint8_t leftPattern = pattern & 0x07;
  uint8_t rightPattern = (pattern >> 3) & 0x07;
  rightPattern = ((rightPattern & 0x01) << 2)
               | (rightPattern & 0x02)
               | ((rightPattern & 0x04) >> 2);
  writeLeftServo(module, CAM_ANGLES[leftPattern]);
  delay(SERVO_SETTLE_MS);
  writeRightServo(module, RIGHT_CAM_ANGLES[rightPattern]);
  delay(SERVO_SETTLE_MS);
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
}

void publishStatus(const char* eventName = "status", const char* messageId = "") {
  if (!mqtt.connected()) return;
  JsonDocument doc;
  doc["module"] = MODULE_ID;
  doc["online"] = true;
  doc["busy"] = isBusy();
  doc["event"] = eventName;
  if (messageId[0] != '\0') doc["message_id"] = messageId;
  doc["rssi"] = WiFi.RSSI();
  JsonArray modules = doc["modules_ok"].to<JsonArray>();
  for (int module = 0; module < NUM_MODULES; module++) modules.add(moduleOK[module]);
  char buffer[384];
  size_t length = serializeJson(doc, buffer, sizeof(buffer));
  mqtt.publish(TOPIC_STATUS, reinterpret_cast<uint8_t*>(buffer), length, false);
}

bool enqueueMessage(JsonArray patterns, const char* messageId) {
  if (queueCount >= QUEUE_SIZE) return false;
  BrailleMessage& slot = messageQueue[queueTail];
  slot.length = 0;
  strlcpy(slot.messageId, messageId, sizeof(slot.messageId));
  for (JsonVariant value : patterns) {
    if (slot.length >= MAX_PATTERNS) break;
    int pattern = value.as<int>();
    if (pattern >= 0 && pattern <= 63) {
      slot.patterns[slot.length++] = static_cast<uint8_t>(pattern);
    }
  }
  if (slot.length == 0) return false;
  queueTail = (queueTail + 1) % QUEUE_SIZE;
  queueCount++;
  return true;
}

bool dequeueMessage() {
  if (queueCount == 0) return false;
  currentMessage = messageQueue[queueHead];
  queueHead = (queueHead + 1) % QUEUE_SIZE;
  queueCount--;
  currentOffset = 0;
  return true;
}

void showNextBatch() {
  Serial.print("Displaying cells: ");
  for (int module = 0; module < NUM_MODULES; module++) {
    uint8_t pattern = 0;
    if (currentOffset < currentMessage.length) pattern = currentMessage.patterns[currentOffset++];
    if (moduleOK[module]) driveModule(module, pattern);
    Serial.print(pattern);
    Serial.print(module == NUM_MODULES - 1 ? '\n' : ' ');
  }
  displayState = DISPLAY_HOLDING;
  stateDeadline = millis() + CHAR_HOLD_TIME;
}

void updateDisplay() {
  if (displayState == DISPLAY_IDLE) {
    if (dequeueMessage()) {
      publishStatus("display_started", currentMessage.messageId);
      showNextBatch();
    }
    return;
  }
  if (static_cast<long>(millis() - stateDeadline) < 0) return;
  if (displayState == DISPLAY_HOLDING) {
    homeAllServos();
    if (currentOffset >= currentMessage.length) {
      displayState = DISPLAY_IDLE;
      publishStatus("display_complete", currentMessage.messageId);
      Serial.println("Display complete.");
    } else {
      displayState = DISPLAY_GAP;
      stateDeadline = millis() + BATCH_GAP_TIME;
    }
  } else if (displayState == DISPLAY_GAP) {
    showNextBatch();
  }
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  if (String(topic) != TOPIC_DISPLAY) return;
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, payload, length);
  if (error) {
    Serial.print("Invalid JSON: ");
    Serial.println(error.c_str());
    return;
  }
  JsonArray patterns = doc["braille"].as<JsonArray>();
  if (patterns.isNull()) {
    Serial.println("Message has no braille array.");
    return;
  }
  const char* messageId = doc["message_id"] | "unknown";
  if (enqueueMessage(patterns, messageId)) {
    Serial.print("Queued message: ");
    Serial.println(messageId);
    publishStatus("queued", messageId);
  } else {
    Serial.println("Queue full or empty message; rejected.");
    publishStatus("queue_rejected", messageId);
  }
}

void connectWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ABILITY_WIFI_SSID, ABILITY_WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print('.');
  }
  Serial.print("\nWi-Fi connected. IP: ");
  Serial.println(WiFi.localIP());
}

void syncClock() {
  configTime(0, 0, "pool.ntp.org", "time.google.com");
  Serial.print("Synchronizing clock");
  time_t now = time(nullptr);
  while (now < 1700000000) {
    delay(500);
    Serial.print('.');
    now = time(nullptr);
  }
  Serial.println(" synchronized.");
}

void connectMQTT() {
  if (mqtt.connected() || WiFi.status() != WL_CONNECTED) return;
  if (millis() - lastMqttAttempt < RECONNECT_INTERVAL) return;
  lastMqttAttempt = millis();
  String clientId = String(MODULE_ID) + "-" + String((uint32_t)ESP.getEfuseMac(), HEX);
  const char* offline = "{\"module\":\"braille_01\",\"online\":false}";
  Serial.print("Connecting securely to EMQX...");
  bool connected = mqtt.connect(
    clientId.c_str(), ABILITY_MQTT_USERNAME, ABILITY_MQTT_PASSWORD,
    TOPIC_STATUS, 1, true, offline
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

void setupServos() {
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);
  Servo* left[NUM_MODULES] = {&m1Left, &m2Left, &m3Left, &m4Left};
  Servo* right[NUM_MODULES] = {&m1Right, &m2Right, &m3Right, &m4Right};
  const int leftPins[NUM_MODULES] = {M1_LEFT_PIN, M2_LEFT_PIN, M3_LEFT_PIN, M4_LEFT_PIN};
  const int rightPins[NUM_MODULES] = {M1_RIGHT_PIN, M2_RIGHT_PIN, M3_RIGHT_PIN, M4_RIGHT_PIN};
  for (int module = 0; module < NUM_MODULES; module++) {
    left[module]->setPeriodHertz(50);
    right[module]->setPeriodHertz(50);
    left[module]->attach(leftPins[module], SERVO_MIN_US, SERVO_MAX_US);
    right[module]->attach(rightPins[module], SERVO_MIN_US, SERVO_MAX_US);
    moduleOK[module] = left[module]->attached() && right[module]->attached();
  }
  homeAllServos();
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\nProject Ability - Braille Cloud Receiver");
  setupServos();
  connectWiFi();
  syncClock();
  secureClient.setCACert(EMQX_ROOT_CA);
  mqtt.setServer(MQTT_SERVER, MQTT_PORT);
  mqtt.setCallback(mqttCallback);
  mqtt.setBufferSize(2048);
  mqtt.setKeepAlive(30);
  lastMqttAttempt = millis() - RECONNECT_INTERVAL;
  connectMQTT();
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    connectWiFi();
    syncClock();
  }
  connectMQTT();
  mqtt.loop();
  updateDisplay();
  if (millis() - lastStatusTime >= STATUS_INTERVAL) {
    publishStatus();
    lastStatusTime = millis();
  }
  delay(5);
}
