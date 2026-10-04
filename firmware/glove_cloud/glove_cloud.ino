/*
 * Project Ability — Secure Cloud Glove
 *
 * Sensor pins, calibration, confirmation timing, debug output, and the
 * complete classifier are kept from glove_flex_imu/glove_flex_imu.ino.
 * Cloud-only additions are TLS MQTT presence/input/output plus an idle send
 * boundary because the reference glove has no physical SEND button.
 *
 * A recognized sentence is published after the hand is neutral for 2.5 s.
 * Serial commands: "send" publishes immediately; "clear" clears the buffer.
 */

#include <Wire.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <time.h>

#include "../braille_cloud/emqx_ca.h"
#include "../braille_cloud/secrets.h"

// Secure cloud MQTT — same broker and credentials as the Braille module.
const char* MQTT_SERVER = "z91cfe11.ala.asia-southeast1.emqxsl.com";
const int   MQTT_PORT   = 8883;
const char* MODULE_ID   = "glove_01";

const char* TOPIC_TEXT   = "ability/v1/glove_01/input";
const char* TOPIC_STATUS = "ability/v1/glove_01/status";
const char* TOPIC_OUTPUT = "ability/v1/glove_01/output";

#define STATUS_INTERVAL 10000UL
#define WIFI_RETRY_INTERVAL 10000UL
#define MQTT_RETRY_INTERVAL 5000UL
#define GLOVE_SEND_IDLE_MS 2500UL

// Flex sensor pins
const int FLEX_PIN[5] = {1, 2, 3, 4, 5};
const char* FNAME[5] = {"Th", "Ix", "Md", "Rg", "Pk"};

// BNO055 raw I2C
#define BNO_ADDR  0x28
#define BNO_CHIP  0x00
#define BNO_OPR   0x3D
#define BNO_PWR   0x3E
#define BNO_TRIG  0x3F
#define BNO_EULER 0x1A

#define TH 0
#define IX 1
#define MD 2
#define RG 3
#define PK 4

bool imuOK = false;
int rawMV[5];
int bend[5];
int calHi[5] = {0, 0, 0, 0, 0};
int calLo[5] = {3300, 3300, 3300, 3300, 3300};
bool autoCalDone = false;
unsigned long autoCalStart = 0;

float heading = 0, roll_a = 0, pitch_a = 0;

String lastG = "";
String confirmedG = "";
unsigned long lastGT = 0;
int gCount = 0;

String sentence = "";

WiFiClientSecure secureClient;
PubSubClient mqtt(secureClient);

unsigned long lastStatusTime = 0;
unsigned long lastWiFiAttempt = 0;
unsigned long lastMqttAttempt = 0;
unsigned long lastConfirmedAt = 0;
bool clockStarted = false;

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

void connectWiFi() {
  if (WiFi.status() == WL_CONNECTED ||
      millis() - lastWiFiAttempt < WIFI_RETRY_INTERVAL) return;
  lastWiFiAttempt = millis();
  Serial.print("[Cloud] WiFi connecting to ");
  Serial.println(ABILITY_WIFI_SSID);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ABILITY_WIFI_SSID, ABILITY_WIFI_PASSWORD);
}

void connectMQTT() {
  if (mqtt.connected() || WiFi.status() != WL_CONNECTED ||
      time(nullptr) < 1700000000 ||
      millis() - lastMqttAttempt < MQTT_RETRY_INTERVAL) return;
  lastMqttAttempt = millis();

  String clientId = String(MODULE_ID) + "-" +
                    String((uint32_t)ESP.getEfuseMac(), HEX);
  const char *offline = "{\"module\":\"glove_01\",\"online\":false}";
  Serial.print("[Cloud] MQTT connecting...");
  bool connected = mqtt.connect(
      clientId.c_str(), ABILITY_MQTT_USERNAME, ABILITY_MQTT_PASSWORD,
      TOPIC_STATUS, 1, true, offline);
  if (connected) {
    Serial.println("connected");
    mqtt.subscribe(TOPIC_OUTPUT, 1);
    publishStatus();
  } else {
    Serial.print("failed, state=");
    Serial.println(mqtt.state());
  }
}

void maintainCloudConnection() {
  if (WiFi.status() != WL_CONNECTED) {
    connectWiFi();
    return;
  }
  if (!clockStarted) {
    configTime(0, 0, "pool.ntp.org", "time.google.com");
    clockStarted = true;
  }
  connectMQTT();
  if (mqtt.connected()) mqtt.loop();
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  char msg[512];
  int len = min((unsigned int)511, length);
  memcpy(msg, payload, len);
  msg[len] = '\0';
  Serial.print("MQTT [");
  Serial.print(topic);
  Serial.print("]: ");
  Serial.println(msg);
}

void publishText(String text) {
  if (!mqtt.connected()) return;
  text.trim();
  if (text.length() == 0) return;

  JsonDocument doc;
  doc["version"] = 1;
  doc["message_id"] = String(MODULE_ID) + "-" + String(millis());
  doc["source"] = MODULE_ID;
  doc["type"] = "text";
  doc["text"] = text;
  doc["timestamp"] = static_cast<unsigned long>(time(nullptr));

  char buffer[256];
  serializeJson(doc, buffer);
  mqtt.publish(TOPIC_TEXT, buffer);

  Serial.print("[Cloud] Published: ");
  Serial.println(text);
}

void publishStatus() {
  if (!mqtt.connected()) return;

  JsonDocument doc;
  doc["module"] = MODULE_ID;
  doc["online"] = true;
  doc["imu_ok"] = imuOK;
  doc["calibrated"] = autoCalDone;
  doc["sentence"] = sentence;
  doc["ip"] = WiFi.localIP().toString();
  doc["rssi"] = WiFi.RSSI();
  doc["timestamp"] = static_cast<unsigned long>(time(nullptr));

  char buffer[256];
  serializeJson(doc, buffer);
  mqtt.publish(TOPIC_STATUS, buffer, true);
}

void setup() {
  Serial.begin(115200);
  unsigned long s = millis();
  while (!Serial && (millis() - s < 3000));

  analogReadResolution(12);
  analogSetAttenuation(ADC_11db);

  Serial.println("\n--- Glove-to-Language (Cloud MQTT) ---");

  for (int i = 0; i < 5; i++) {
    Serial.print(FNAME[i]);
    Serial.print(":");
    Serial.print(analogReadMilliVolts(FLEX_PIN[i]));
    Serial.print("mV  ");
  }
  Serial.println();

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

  // Cloud transport is asynchronous so calibration/recognition never blocks.
  secureClient.setCACert(EMQX_ROOT_CA);
  mqtt.setServer(MQTT_SERVER, MQTT_PORT);
  mqtt.setCallback(mqttCallback);
  mqtt.setBufferSize(1024);
  mqtt.setKeepAlive(30);
  lastWiFiAttempt = millis() - WIFI_RETRY_INTERVAL;
  connectWiFi();

  Serial.println("Move fingers flat<->bent to calibrate\n");
  autoCalStart = millis();
}

void loop() {
  maintainCloudConnection();

  // Read flex sensors
  for (int i = 0; i < 5; i++) {
    rawMV[i] = analogReadMilliVolts(FLEX_PIN[i]);
    if (rawMV[i] > calHi[i]) calHi[i] = rawMV[i];
    if (rawMV[i] < calLo[i]) calLo[i] = rawMV[i];
    if (calHi[i] != calLo[i]) {
      bend[i] = map(rawMV[i], calHi[i], calLo[i], 0, 100);
      bend[i] = constrain(bend[i], 0, 100);
    }
  }

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

  // Classify gesture
  String g = classify();

  // Gesture confirmation (hold 4 cycles = ~400ms)
  if (g != "---") {
    if (g == lastG) {
      gCount++;
      if (gCount == 4 && g != confirmedG) {
        confirmedG = g;
        sentence += g;
        lastConfirmedAt = millis();

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

  // Debug — deliberately matches glove_flex_imu.ino.
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

  // The base glove has no send button. End a cloud message after the hand
  // returns to neutral and no new confirmed sign arrives for 2.5 seconds.
  if (mqtt.connected() && sentence.length() > 0 && g == "---" && lastConfirmedAt > 0 &&
      millis() - lastConfirmedAt >= GLOVE_SEND_IDLE_MS) {
    publishText(sentence);
    sentence = "";
    lastConfirmedAt = 0;
  }

  // Periodic status
  if (millis() - lastStatusTime > STATUS_INTERVAL) {
    publishStatus();
    lastStatusTime = millis();
  }

  // Serial commands for debugging
  if (Serial.available() > 0) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();
    if (cmd.equalsIgnoreCase("send") && sentence.length() > 0) {
      publishText(sentence);
      sentence = "";
    } else if (cmd.equalsIgnoreCase("clear")) {
      sentence = "";
      Serial.println("Sentence cleared.");
    } else if (cmd.length() > 0) {
      // Type text directly to send via MQTT (for testing)
      publishText(cmd);
    }
  }

  delay(100);
}

// ASL Classifier — same as working version
String classify() {
  bool o[5], h[5], b[5];
  for (int i = 0; i < 5; i++) {
    o[i] = bend[i] < 30;
    h[i] = bend[i] >= 30 && bend[i] < 60;
    b[i] = bend[i] >= 60;
  }

  bool palmFwd = true, palmDown = false, palmUp = false, palmSide = false;
  if (imuOK) {
    palmFwd  = pitch_a > -30 && pitch_a < 30;
    palmDown = pitch_a <= -30;
    palmUp   = pitch_a >= 60;
    palmSide = (roll_a > 40 && roll_a < 140) || (roll_a < -40 && roll_a > -140);
  }

  if (b[TH]&&b[IX]&&b[MD]&&b[RG]&&b[PK]) return "S";
  if (o[TH]&&o[IX]&&o[MD]&&o[RG]&&o[PK]) {
    if (imuOK && (palmDown||palmUp)) return "5";
    return "B";
  }
  if (o[TH]&&b[IX]&&b[MD]&&b[RG]&&b[PK]) return "A";
  if (b[TH]&&o[IX]&&b[MD]&&b[RG]&&b[PK]) {
    if (imuOK && palmSide) return "G";
    if (imuOK && palmDown) return "Q";
    return "D";
  }
  if (b[TH]&&h[IX]&&b[MD]&&b[RG]&&b[PK]) return "X";
  if (o[TH]&&o[IX]&&b[MD]&&b[RG]&&b[PK]) return "L";
  if (b[TH]&&o[IX]&&o[MD]&&b[RG]&&b[PK]) {
    if (imuOK && palmSide) return "H";
    if (imuOK && palmDown) return "P";
    return "V";
  }
  if (o[TH]&&o[IX]&&o[MD]&&b[RG]&&b[PK]) return "3";
  if (h[TH]&&o[IX]&&o[MD]&&b[RG]&&b[PK]) return "K";
  if (b[TH]&&o[IX]&&o[MD]&&o[RG]&&b[PK]) return "W";
  if (b[TH]&&o[IX]&&o[MD]&&o[RG]&&o[PK]) return "9";
  if (o[TH]&&o[IX]&&o[MD]&&o[RG]&&b[PK]) return "4";
  if (b[TH]&&b[IX]&&b[MD]&&b[RG]&&o[PK]) return "I";
  if (o[TH]&&b[IX]&&b[MD]&&b[RG]&&o[PK]) return "Y";
  if (o[TH]&&o[IX]&&b[MD]&&b[RG]&&o[PK]) return "ILY";
  if (b[TH]&&o[IX]&&b[MD]&&b[RG]&&o[PK]) return "U";
  if (b[TH]&&b[IX]&&b[MD]&&o[RG]&&o[PK]) return "6";
  if (b[TH]&&b[IX]&&o[MD]&&b[RG]&&b[PK]) return "8";
  if (b[TH]&&b[IX]&&o[MD]&&o[RG]&&o[PK]) return "7";
  if (h[TH]&&h[IX]&&h[MD]&&h[RG]&&h[PK]) return "C";
  if (h[TH]&&h[IX]&&b[MD]&&b[RG]&&b[PK]) return "O";
  if (h[TH]&&h[IX]&&h[MD]&&h[RG]&&b[PK]) return "E";
  if (h[TH]&&h[IX]&&o[MD]&&o[RG]&&o[PK]) return "F";
  if (b[TH]&&h[IX]&&h[MD]&&b[RG]&&b[PK]) return "R";
  if (h[TH]&&b[IX]&&b[MD]&&b[RG]&&b[PK]) return "T";
  if (h[TH]&&o[IX]&&b[MD]&&b[RG]&&b[PK]) return "1";
  if (h[TH]&&h[IX]&&o[MD]&&b[RG]&&b[PK]) return "N";
  if (h[TH]&&o[IX]&&o[MD]&&o[RG]&&b[PK]) return "M";

  return "---";
}
