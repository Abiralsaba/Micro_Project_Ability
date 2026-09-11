/*
 * ══════════════════════════════════════════════════════════════
 * GLOVE MODULE FIRMWARE — Project Ability
 * Target: ESP32-S3 WROOM (Arduino IDE)
 * Board: "ESP32S3 Dev Module" in Arduino IDE
 * ══════════════════════════════════════════════════════════════
 *
 * PURPOSE:
 *   Reads sensor glove data (flex sensors, IMU, environment,
 *   health sensors) and streams it to the local RPi Zero 2W
 *   via Serial UART.
 *   The RPi Zero handles camera + MediaPipe + ML fusion → TEXT.
 *
 * SENSORS:
 *   - 5× Flex Sensors via ADS1115 (I²C, addr 0x48)
 *   - BNO055 IMU (I²C, addr 0x28) — palm orientation (quaternion + Euler)
 *   - BMP280 (I²C, addr 0x76/0x77) — barometric pressure + temperature
 *   - MAX30102 (I²C, addr 0x57) — SpO2 + heart rate
 *   - MPU6050 (I²C, addr 0x68) — fall detection (runs on Core 1)
 *
 * COMMUNICATION:
 *   - Serial UART → RPi Zero 2W (sensor data at 100 Hz)
 *   - WiFi → Central Hub MQTT (health data, status)
 *
 * LIBRARIES REQUIRED (install via Arduino IDE Library Manager):
 *   - Adafruit ADS1X15
 *   - Adafruit BNO055
 *   - Adafruit Unified Sensor
 *   - Adafruit BMP280
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
#include <Adafruit_ADS1X15.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <utility/imumaths.h>
#include <Adafruit_BMP280.h>
#include "MAX30105.h"           // SparkFun MAX3010x library
#include "heartRate.h"          // SparkFun heart rate algorithm
#include <Adafruit_MPU6050.h>

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

// Battery ADC pin (voltage divider to LiPo)
#define BATTERY_PIN 4

// ── Sensor Objects ──────────────────────────────────────────
Adafruit_ADS1115 ads;                                // 16-bit ADC for flex sensors
Adafruit_BNO055  bno = Adafruit_BNO055(55, 0x28);   // IMU — palm orientation
Adafruit_BMP280  bmp;                                // Barometric pressure + temperature
MAX30105         particleSensor;                     // SpO2 + heart rate
Adafruit_MPU6050 mpu;                                // Fall detection (secondary IMU)

// ── Global Objects ──────────────────────────────────────────
WiFiClient   wifiClient;
PubSubClient mqttClient(wifiClient);

// ── Timing ──────────────────────────────────────────────────
unsigned long lastSensorRead   = 0;
unsigned long lastHealthReport = 0;
unsigned long lastEnvReport    = 0;
const int     SENSOR_INTERVAL  = 10;     // 100 Hz sensor streaming
const int     HEALTH_INTERVAL  = 5000;   // Health data every 5 seconds
const int     ENV_INTERVAL     = 10000;  // Environment data every 10 seconds

// ── Sensor Status Flags ─────────────────────────────────────
bool adsReady  = false;
bool bnoReady  = false;
bool bmpReady  = false;
bool maxReady  = false;
bool mpuReady  = false;

// ── BNO055 Calibration Storage ──────────────────────────────
// BNO055 loses calibration on power cycle — we save/restore offsets
bool bnoCalibrated = false;

// ── Heart Rate Variables ────────────────────────────────────
const byte HR_RATE_SIZE = 4;
byte       hrRates[HR_RATE_SIZE];
byte       hrRateSpot = 0;
long       lastHrBeat = 0;
float      beatsPerMinute = 0;
int        beatAvg = 0;

// ── Fall Detection Variables (MPU6050 on Core 1) ────────────
volatile bool fallDetected = false;
const float   FALL_THRESHOLD = 2.5;  // g-force threshold for fall event

// ── Fall Detection Task Handle ──────────────────────────────
TaskHandle_t fallDetectionTaskHandle = NULL;

// ══════════════════════════════════════════════════════════════
// SETUP
// ══════════════════════════════════════════════════════════════

void setup() {
  // Debug serial
  Serial.begin(115200);
  Serial.println("[Ability] Glove Module — Initializing...");

  // Serial to RPi Zero 2W
  Serial1.begin(115200, SERIAL_8N1, ZERO_RX, ZERO_TX);

  // I²C bus
  Wire.begin(I2C_SDA, I2C_SCL);
  Wire.setClock(400000);  // 400 kHz Fast Mode for all I²C sensors

  // ── Initialize ADS1115 (flex sensors) ──────────────────────
  if (ads.begin(0x48)) {
    ads.setGain(GAIN_ONE);        // ±4.096V range (flex sensors are 0–3.3V)
    ads.setDataRate(RATE_ADS1115_250SPS);  // 250 samples/sec — fast enough
    adsReady = true;
    Serial.println("[ADS1115] ✓ Flex sensor ADC initialized");
  } else {
    Serial.println("[ADS1115] ✗ FAILED — flex sensors unavailable");
  }

  // ── Initialize BNO055 (IMU — palm orientation) ─────────────
  if (bno.begin()) {
    bno.setExtCrystalUse(true);   // Use external 32.768 kHz crystal for better accuracy
    // Set to NDOF mode (9-DOF absolute orientation fusion)
    // This is the default, but let's be explicit
    bno.setMode(Adafruit_BNO055::OPERATION_MODE_NDOF);
    bnoReady = true;
    Serial.println("[BNO055]  ✓ IMU initialized (NDOF fusion mode)");
    Serial.println("[BNO055]  ℹ Calibration will auto-converge — wave hand slowly");
  } else {
    Serial.println("[BNO055]  ✗ FAILED — palm orientation unavailable");
  }

  // ── Initialize BMP280 (barometric pressure + temperature) ──
  if (bmp.begin(0x76)) {
    // Oversampling for stable indoor readings
    bmp.setSampling(
      Adafruit_BMP280::MODE_NORMAL,      // Continuous measurement
      Adafruit_BMP280::SAMPLING_X2,      // Temperature: 2× oversampling
      Adafruit_BMP280::SAMPLING_X16,     // Pressure: 16× oversampling
      Adafruit_BMP280::FILTER_X16,       // IIR filter: 16× (smooth)
      Adafruit_BMP280::STANDBY_MS_500    // 500ms standby between readings
    );
    bmpReady = true;
    Serial.println("[BMP280]  ✓ Barometric sensor initialized");
  } else {
    // Try alternate address 0x77
    if (bmp.begin(0x77)) {
      bmp.setSampling(
        Adafruit_BMP280::MODE_NORMAL,
        Adafruit_BMP280::SAMPLING_X2,
        Adafruit_BMP280::SAMPLING_X16,
        Adafruit_BMP280::FILTER_X16,
        Adafruit_BMP280::STANDBY_MS_500
      );
      bmpReady = true;
      Serial.println("[BMP280]  ✓ Barometric sensor initialized (addr 0x77)");
    } else {
      Serial.println("[BMP280]  ✗ FAILED — environment data unavailable");
    }
  }

  // ── Initialize MAX30102 (SpO2 + heart rate) ────────────────
  if (particleSensor.begin(Wire, I2C_SPEED_FAST)) {
    particleSensor.setup();                // Default settings
    particleSensor.setPulseAmplitudeRed(0x0A);   // Low power — just detecting
    particleSensor.setPulseAmplitudeGreen(0);     // Green LED off
    maxReady = true;
    Serial.println("[MAX30102]✓ SpO2 / heart rate initialized");
  } else {
    Serial.println("[MAX30102]✗ FAILED — health monitoring unavailable");
  }

  // ── Initialize MPU6050 (fall detection) ────────────────────
  if (mpu.begin(0x68)) {
    mpu.setAccelerometerRange(MPU6050_RANGE_8_G);   // ±8g for detecting impacts
    mpu.setGyroRange(MPU6050_RANGE_500_DEG);
    mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);     // Low-pass filter
    mpuReady = true;
    Serial.println("[MPU6050] ✓ Fall detection initialized");

    // Launch fall detection on Core 1 (separate from main loop on Core 0)
    xTaskCreatePinnedToCore(
      fallDetectionTask,         // Task function
      "FallDetection",           // Name
      4096,                      // Stack size (bytes)
      NULL,                      // Parameters
      1,                         // Priority
      &fallDetectionTaskHandle,  // Task handle
      1                          // Core 1
    );
    Serial.println("[MPU6050] ℹ Fall detection running on Core 1");
  } else {
    Serial.println("[MPU6050] ✗ FAILED — fall detection unavailable");
  }

  // ── WiFi connection ────────────────────────────────────────
  connectWiFi();

  // ── MQTT connection to Central Hub ─────────────────────────
  mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
  mqttClient.setCallback(mqttCallback);
  connectMQTT();

  // ── Print sensor summary ──────────────────────────────────
  Serial.println("\n[Ability] ═══ Sensor Status ═══");
  Serial.printf("  ADS1115 (Flex):   %s\n", adsReady ? "✓ OK" : "✗ FAIL");
  Serial.printf("  BNO055  (IMU):    %s\n", bnoReady ? "✓ OK" : "✗ FAIL");
  Serial.printf("  BMP280  (Baro):   %s\n", bmpReady ? "✓ OK" : "✗ FAIL");
  Serial.printf("  MAX30102 (SpO2):  %s\n", maxReady ? "✓ OK" : "✗ FAIL");
  Serial.printf("  MPU6050 (Fall):   %s\n", mpuReady ? "✓ OK" : "✗ FAIL");
  Serial.println("[Ability] Glove Module — Ready!\n");
}

// ══════════════════════════════════════════════════════════════
// MAIN LOOP
// ══════════════════════════════════════════════════════════════

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

  // ── Periodic environment data to Central Hub (every 10s) ──
  if (now - lastEnvReport >= ENV_INTERVAL) {
    lastEnvReport = now;
    reportEnvironmentData();
  }

  // ── Update heart rate (non-blocking) ──────────────────────
  if (maxReady) {
    updateHeartRate();
  }
}

// ══════════════════════════════════════════════════════════════
// SENSOR READING & STREAMING (100 Hz → RPi Zero)
// ══════════════════════════════════════════════════════════════

void readAndStreamSensors() {
  JsonDocument doc;
  doc["ts"] = millis();

  // ── Read 5 flex sensor values from ADS1115 ────────────────
  // ADS1115 has 4 channels; we read 5 flex sensors using
  // channels 0-3 (4 fingers) + one via differential or
  // a second ADS1115. For now, 4 channels + 1 raw ADC pin.
  if (adsReady) {
    doc["f"][0] = ads.readADC_SingleEnded(0);  // Thumb
    doc["f"][1] = ads.readADC_SingleEnded(1);  // Index
    doc["f"][2] = ads.readADC_SingleEnded(2);  // Middle
    doc["f"][3] = ads.readADC_SingleEnded(3);  // Ring
    doc["f"][4] = analogRead(5);               // Pinky (ESP32 ADC pin fallback)
  }

  // ── Read orientation from BNO055 ──────────────────────────
  if (bnoReady) {
    // Quaternion — primary output for ML model (drift-free, no gimbal lock)
    imu::Quaternion quat = bno.getQuat();
    doc["q"][0] = quat.w();
    doc["q"][1] = quat.x();
    doc["q"][2] = quat.y();
    doc["q"][3] = quat.z();

    // Euler angles — human-readable backup (heading, roll, pitch)
    sensors_event_t orientationData;
    bno.getEvent(&orientationData, Adafruit_BNO055::VECTOR_EULER);
    doc["e"][0] = orientationData.orientation.x;  // Heading (yaw) 0–360°
    doc["e"][1] = orientationData.orientation.y;  // Roll ±180°
    doc["e"][2] = orientationData.orientation.z;  // Pitch ±90°

    // Linear acceleration (gravity removed) — useful for gesture velocity
    sensors_event_t linearAccelData;
    bno.getEvent(&linearAccelData, Adafruit_BNO055::VECTOR_LINEARACCEL);
    doc["la"][0] = linearAccelData.acceleration.x;
    doc["la"][1] = linearAccelData.acceleration.y;
    doc["la"][2] = linearAccelData.acceleration.z;

    // Calibration status (0=uncalibrated, 3=fully calibrated)
    uint8_t sysCal, gyroCal, accelCal, magCal;
    bno.getCalibration(&sysCal, &gyroCal, &accelCal, &magCal);
    doc["cal"] = sysCal;  // System calibration (0-3)

    // Mark calibrated once system reaches level 3
    if (sysCal == 3 && !bnoCalibrated) {
      bnoCalibrated = true;
      Serial.println("[BNO055]  ✓ Fully calibrated!");
    }
  }

  // ── Stream JSON to RPi Zero 2W via Serial UART ────────────
  serializeJson(doc, Serial1);
  Serial1.println();  // Newline delimiter for RPi parser
}

// ══════════════════════════════════════════════════════════════
// HEART RATE UPDATE (called every loop iteration, non-blocking)
// ══════════════════════════════════════════════════════════════

void updateHeartRate() {
  long irValue = particleSensor.getIR();

  // Only process if finger is actually on the sensor
  if (irValue < 50000) return;  // No finger detected

  if (checkForBeat(irValue)) {
    long delta = millis() - lastHrBeat;
    lastHrBeat = millis();

    beatsPerMinute = 60.0 / (delta / 1000.0);

    // Sanity check — valid HR range
    if (beatsPerMinute > 20 && beatsPerMinute < 255) {
      hrRates[hrRateSpot++ % HR_RATE_SIZE] = (byte)beatsPerMinute;

      // Compute running average
      beatAvg = 0;
      for (byte i = 0; i < HR_RATE_SIZE; i++) {
        beatAvg += hrRates[i];
      }
      beatAvg /= HR_RATE_SIZE;
    }
  }
}

// ══════════════════════════════════════════════════════════════
// HEALTH MONITORING (every 5s → MQTT to Central Hub)
// ══════════════════════════════════════════════════════════════

void reportHealthData() {
  JsonDocument doc;
  doc["module_id"] = MODULE_ID;
  doc["type"]      = "health";

  // ── Heart rate from MAX30102 ──────────────────────────────
  if (maxReady) {
    long irValue = particleSensor.getIR();
    doc["heart_rate"] = beatAvg;
    doc["finger_on"]  = (irValue > 50000);

    // SpO2 reading (simplified — full algorithm would use red + IR ratio)
    doc["spo2"] = 0;  // TODO: implement full SpO2 algorithm with red/IR ratio
  }

  // ── Fall detection status from MPU6050 (Core 1) ───────────
  doc["fall"] = fallDetected;
  if (fallDetected) {
    Serial.println("[ALERT] ⚠ FALL DETECTED — notifying hub!");
    fallDetected = false;  // Reset after reporting
  }

  // ── Battery level ─────────────────────────────────────────
  int rawBattery = analogRead(BATTERY_PIN);
  float batteryVoltage = (rawBattery / 4095.0) * 3.3 * 2.0;  // Assumes 1:1 voltage divider
  int batteryPercent = constrain(map(batteryVoltage * 100, 320, 420, 0, 100), 0, 100);
  doc["battery_v"]   = batteryVoltage;
  doc["battery_pct"] = batteryPercent;

  // ── BNO055 calibration status ─────────────────────────────
  if (bnoReady) {
    uint8_t sysCal, gyroCal, accelCal, magCal;
    bno.getCalibration(&sysCal, &gyroCal, &accelCal, &magCal);
    doc["imu_cal"]["sys"]   = sysCal;
    doc["imu_cal"]["gyro"]  = gyroCal;
    doc["imu_cal"]["accel"] = accelCal;
    doc["imu_cal"]["mag"]   = magCal;
  }

  // ── Publish to MQTT ───────────────────────────────────────
  char buffer[512];
  serializeJson(doc, buffer);
  String topic = "ability/local/" + String(MODULE_ID) + "/health";
  mqttClient.publish(topic.c_str(), buffer);
}

// ══════════════════════════════════════════════════════════════
// ENVIRONMENT DATA (every 10s → MQTT to Central Hub)
// ══════════════════════════════════════════════════════════════

void reportEnvironmentData() {
  if (!bmpReady) return;

  JsonDocument doc;
  doc["module_id"]    = MODULE_ID;
  doc["type"]         = "environment";
  doc["temperature"]  = bmp.readTemperature();     // °C
  doc["pressure"]     = bmp.readPressure() / 100.0; // hPa (mbar)
  doc["altitude"]     = bmp.readAltitude(1013.25);  // Approximate altitude (m)

  char buffer[256];
  serializeJson(doc, buffer);
  String topic = "ability/local/" + String(MODULE_ID) + "/environment";
  mqttClient.publish(topic.c_str(), buffer);
}

// ══════════════════════════════════════════════════════════════
// FALL DETECTION TASK (runs on Core 1, independent of main loop)
// ══════════════════════════════════════════════════════════════

void fallDetectionTask(void* parameter) {
  Serial.println("[Core 1] Fall detection task started");

  while (true) {
    if (mpuReady) {
      sensors_event_t accel, gyro, temp;
      mpu.getEvent(&accel, &gyro, &temp);

      // Calculate total acceleration magnitude
      float totalAccel = sqrt(
        accel.acceleration.x * accel.acceleration.x +
        accel.acceleration.y * accel.acceleration.y +
        accel.acceleration.z * accel.acceleration.z
      ) / 9.81;  // Convert to g-force

      // Simple fall detection: sudden spike > threshold
      // In freefall, total accel drops near 0, then spikes on impact
      if (totalAccel > FALL_THRESHOLD) {
        fallDetected = true;
      }
    }

    vTaskDelay(pdMS_TO_TICKS(20));  // 50 Hz sampling for fall detection
  }
}

// ══════════════════════════════════════════════════════════════
// MQTT CALLBACK (messages from Central Hub)
// ══════════════════════════════════════════════════════════════

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  // Parse incoming messages from Hub (avatar commands, display text, etc.)
  String message = "";
  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }

  Serial.printf("[MQTT] Received on %s: %s\n", topic, message.c_str());

  // Forward to RPi Zero for display/avatar rendering
  Serial1.print("HUB:");
  Serial1.println(message);
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
      // Subscribe to output from Hub (avatar commands, display text)
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
