/*
 * ══════════════════════════════════════════════════════════════
 * GLOVE MODULE FIRMWARE — Project Ability
 * Target: ESP32-S3 WROOM (Arduino IDE)
 * Board: "ESP32S3 Dev Module" in Arduino IDE
 * ══════════════════════════════════════════════════════════════
 *
 * PURPOSE:
 *   Reads sensor glove data (flex sensors, IMU, health sensors)
 *   and streams it to the local RPi Zero 2W via Serial UART.
 *   The RPi Zero handles camera + MediaPipe + ML fusion → TEXT.
 *
 * SENSORS:
 *   - 5× Flex Sensors via ADS1115 (I²C)
 *   - BNO085 IMU (I²C) — palm orientation
 *   - MAX30102 (I²C) — SpO2 + heart rate
 *   - MPU6050 (I²C) — fall detection (runs on Core 1)
 *
 * COMMUNICATION:
 *   - Serial UART → RPi Zero 2W (sensor data at 100+ Hz)
 *   - WiFi → Central Hub MQTT (health data, status)
 *
 * LIBRARIES REQUIRED (install via Arduino IDE Library Manager):
 *   - Adafruit ADS1X15
 *   - Adafruit BNO08x
 *   - SparkFun MAX3010x
 *   - Adafruit MPU6050
 *   - ArduinoJson
 *   - PubSubClient (MQTT)
 *   - WiFi (built-in for ESP32)
 *
 * BOARD SETUP (Arduino IDE):
 *   1. File → Preferences → Additional Board URLs:
 *      https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
 *   2. Tools → Board → ESP32 Arduino → "ESP32S3 Dev Module"
 *   3. Tools → USB CDC On Boot → "Enabled"
 *   4. Tools → Upload Speed → 921600
 *   5. Tools → Flash Size → "8MB (64Mb)"
 */

#include <Wire.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

// ── WiFi & MQTT Configuration ───────────────────────────────
const char* WIFI_SSID     = "YOUR_WIFI_SSID";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";
const char* MQTT_SERVER   = "CENTRAL_HUB_IP";
const int   MQTT_PORT     = 1883;
const char* MODULE_ID     = "glove_user_01";

// ── Pin Definitions ─────────────────────────────────────────
// I²C (shared bus for all sensors)
#define I2C_SDA 21
#define I2C_SCL 22

// Serial UART to RPi Zero 2W
#define ZERO_TX 17
#define ZERO_RX 16

// ── Global Objects ──────────────────────────────────────────
WiFiClient   wifiClient;
PubSubClient mqttClient(wifiClient);

// ── Timing ──────────────────────────────────────────────────
unsigned long lastSensorRead   = 0;
unsigned long lastHealthReport = 0;
const int     SENSOR_INTERVAL  = 10;    // 100 Hz sensor streaming
const int     HEALTH_INTERVAL  = 5000;  // Health data every 5 seconds

void setup() {
  // Debug serial
  Serial.begin(115200);
  Serial.println("[Ability] Glove Module — Initializing...");

  // Serial to RPi Zero 2W
  Serial1.begin(115200, SERIAL_8N1, ZERO_RX, ZERO_TX);

  // I²C bus
  Wire.begin(I2C_SDA, I2C_SCL);

  // TODO: Initialize ADS1115 (flex sensors)
  // TODO: Initialize BNO085 (IMU)
  // TODO: Initialize MAX30102 (SpO2)
  // TODO: Initialize MPU6050 (fall detection — on Core 1)

  // WiFi connection
  connectWiFi();

  // MQTT connection to Central Hub
  mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
  connectMQTT();

  Serial.println("[Ability] Glove Module — Ready!");
}

void loop() {
  // Maintain MQTT connection
  if (!mqttClient.connected()) {
    connectMQTT();
  }
  mqttClient.loop();

  unsigned long now = millis();

  // ── High-frequency sensor streaming to RPi Zero (100 Hz) ──
  if (now - lastSensorRead >= SENSOR_INTERVAL) {
    lastSensorRead = now;
    readAndStreamSensors();
  }

  // ── Periodic health data to Central Hub (every 5s) ────────
  if (now - lastHealthReport >= HEALTH_INTERVAL) {
    lastHealthReport = now;
    reportHealthData();
  }
}

// ══════════════════════════════════════════════════════════════
// SENSOR READING & STREAMING
// ══════════════════════════════════════════════════════════════

void readAndStreamSensors() {
  // TODO: Read 5 flex sensor values from ADS1115
  // TODO: Read quaternion from BNO085
  // TODO: Package as JSON and send via Serial1 to RPi Zero

  /*
  JsonDocument doc;
  doc["ts"]   = millis();
  doc["f"][0] = flex0;  // Thumb
  doc["f"][1] = flex1;  // Index
  doc["f"][2] = flex2;  // Middle
  doc["f"][3] = flex3;  // Ring
  doc["f"][4] = flex4;  // Pinky
  doc["q"][0] = qw;     // Quaternion W
  doc["q"][1] = qx;     // Quaternion X
  doc["q"][2] = qy;     // Quaternion Y
  doc["q"][3] = qz;     // Quaternion Z

  serializeJson(doc, Serial1);
  Serial1.println();
  */
}

// ══════════════════════════════════════════════════════════════
// HEALTH MONITORING
// ══════════════════════════════════════════════════════════════

void reportHealthData() {
  // TODO: Read SpO2 + heart rate from MAX30102
  // TODO: Check fall detection from MPU6050
  // TODO: Publish to MQTT topic

  /*
  JsonDocument doc;
  doc["module_id"] = MODULE_ID;
  doc["spo2"]      = spo2Value;
  doc["heart_rate"] = heartRate;
  doc["fall"]      = fallDetected;
  doc["battery"]   = analogRead(BATTERY_PIN);

  char buffer[256];
  serializeJson(doc, buffer);
  String topic = "ability/local/" + String(MODULE_ID) + "/health";
  mqttClient.publish(topic.c_str(), buffer);
  */
}

// ══════════════════════════════════════════════════════════════
// CONNECTIVITY
// ══════════════════════════════════════════════════════════════

void connectWiFi() {
  Serial.print("[WiFi] Connecting to ");
  Serial.println(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n[WiFi] Connected! IP: " + WiFi.localIP().toString());
}

void connectMQTT() {
  while (!mqttClient.connected()) {
    Serial.print("[MQTT] Connecting to Central Hub...");
    if (mqttClient.connect(MODULE_ID)) {
      Serial.println(" Connected!");
      // Subscribe to output from Hub (avatar commands)
      String topic = "ability/local/" + String(MODULE_ID) + "/output";
      mqttClient.subscribe(topic.c_str());
    } else {
      Serial.print(" Failed (rc=");
      Serial.print(mqttClient.state());
      Serial.println("). Retrying in 5s...");
      delay(5000);
    }
  }
}
