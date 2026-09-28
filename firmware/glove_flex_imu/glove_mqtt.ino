#include <Wire.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

// WiFi — connects to Pi's Access Point
const char* WIFI_SSID = "Ability";
const char* WIFI_PASS = "ability123";

// MQTT — Pi is the broker
const char* MQTT_SERVER = "192.168.4.1";
const int   MQTT_PORT   = 1883;
const char* MODULE_ID   = "glove_01";

// MQTT Topics
const char* TOPIC_TEXT   = "ability/glove/text";
const char* TOPIC_STATUS = "ability/glove/status";
const char* TOPIC_OUTPUT = "ability/glove/output";

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

WiFiClient wifiClient;
PubSubClient mqtt(wifiClient);

unsigned long lastStatusTime = 0;
#define STATUS_INTERVAL 10000

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
    Serial.println(" FAILED!");
  }
}

void connectMQTT() {
  while (!mqtt.connected()) {
    Serial.print("MQTT: Connecting...");
    if (mqtt.connect(MODULE_ID)) {
      Serial.println(" connected!");
      mqtt.subscribe(TOPIC_OUTPUT);
    } else {
      Serial.print(" failed (rc=");
      Serial.print(mqtt.state());
      Serial.println("). Retry in 3s...");
      delay(3000);
    }
  }
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  char msg[256];
  int len = min((unsigned int)255, length);
  memcpy(msg, payload, len);
  msg[len] = '\0';
  Serial.print("MQTT [");
  Serial.print(topic);
  Serial.print("]: ");
  Serial.println(msg);
}

void publishText(String text) {
  if (!mqtt.connected()) return;

  JsonDocument doc;
  doc["text"] = text;
  doc["user"] = MODULE_ID;
  doc["timestamp"] = millis();

  char buffer[256];
  serializeJson(doc, buffer);
  mqtt.publish(TOPIC_TEXT, buffer);

  Serial.print("MQTT Published: ");
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

  char buffer[256];
  serializeJson(doc, buffer);
  mqtt.publish(TOPIC_STATUS, buffer);
}

void setup() {
  Serial.begin(115200);
  unsigned long s = millis();
  while (!Serial && (millis() - s < 3000));

  analogReadResolution(12);
  analogSetAttenuation(ADC_11db);

  Serial.println("\n--- Glove Module (MQTT) ---");

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

  // WiFi + MQTT
  connectWiFi();
  mqtt.setServer(MQTT_SERVER, MQTT_PORT);
  mqtt.setCallback(mqttCallback);
  connectMQTT();

  Serial.println("Calibrate: move fingers flat<->bent");
  Serial.println();
  autoCalStart = millis();
}

void loop() {
  // Maintain connections
  if (WiFi.status() != WL_CONNECTED) connectWiFi();
  if (!mqtt.connected()) connectMQTT();
  mqtt.loop();

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
    Serial.println("\n--- Calibration Complete ---\n");
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

        // Handle special gestures
        if (g == "SPACE") {
          sentence += " ";
        } else if (g == "SEND") {
          // Send the complete sentence via MQTT
          if (sentence.length() > 0) {
            Serial.println();
            Serial.print(">>> SENDING: [");
            Serial.print(sentence);
            Serial.println("]");
            publishText(sentence);
            sentence = "";
          }
        } else {
          sentence += g;
        }

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

  // Debug output
  Serial.print("BEND[T:");
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
    Serial.print(" Pit:");
    Serial.print(pitch_a, 1);
    Serial.print(" Rol:");
    Serial.print(roll_a, 1);
  }
  Serial.print(" | ");
  Serial.print(g);
  Serial.print(" | [");
  Serial.print(sentence);
  Serial.println("]");

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
    if (imuOK && palmDown) return "SEND";      // Palm down, all open = send sentence
    if (imuOK && palmUp) return "SPACE";        // Palm up, all open = space
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
