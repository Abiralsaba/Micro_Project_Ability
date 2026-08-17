/*
 * ══════════════════════════════════════════════════════════════
 * VOICE MODULE FIRMWARE — Project Ability
 * Target: ESP32-S3 WROOM (Arduino IDE)
 * Board: "ESP32S3 Dev Module" in Arduino IDE
 * ══════════════════════════════════════════════════════════════
 *
 * PURPOSE:
 *   Captures audio from INMP441 I2S mic and streams it to the
 *   Central Hub via WiFi (32 kbps — lighter than a phone call).
 *   The Hub runs STT (Vosk/Whisper) to convert speech → TEXT.
 *   Receives TTS audio from Hub → plays on I2S speaker.
 *
 * HARDWARE:
 *   - INMP441 I2S MEMS Microphone (voice input)
 *   - MAX98357A I2S Amplifier + 3W Speaker (audio output)
 *   - Optional: Small OLED screen (status display)
 *
 * LIBRARIES REQUIRED (install via Arduino IDE Library Manager):
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
#include <driver/i2s.h>

// ── WiFi & MQTT Configuration ───────────────────────────────
const char* WIFI_SSID     = "YOUR_WIFI_SSID";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";
const char* MQTT_SERVER   = "CENTRAL_HUB_IP";
const int   MQTT_PORT     = 1883;
const char* MODULE_ID     = "voice_user_01";

// ── I2S Microphone (INMP441) Pin Definitions ────────────────
#define MIC_I2S_PORT   I2S_NUM_0
#define MIC_I2S_WS     15    // Word Select (LRCLK)
#define MIC_I2S_SCK    14    // Bit Clock (BCLK)
#define MIC_I2S_SD     32    // Serial Data (DOUT)

// ── I2S Speaker (MAX98357A) Pin Definitions ─────────────────
#define SPK_I2S_PORT   I2S_NUM_1
#define SPK_I2S_BCLK   26
#define SPK_I2S_LRC    25
#define SPK_I2S_DOUT   22

// ── Audio Configuration ─────────────────────────────────────
#define SAMPLE_RATE    16000
#define BUFFER_SIZE    1024
#define BITS_PER_SAMPLE I2S_BITS_PER_SAMPLE_16BIT

// ── Global Objects ──────────────────────────────────────────
WiFiClient   wifiClient;
PubSubClient mqttClient(wifiClient);

bool isRecording = false;
int16_t audioBuffer[BUFFER_SIZE];

void setup() {
  Serial.begin(115200);
  Serial.println("[Ability] Voice Module — Initializing...");

  // Initialize I2S Microphone
  setupMicI2S();

  // Initialize I2S Speaker
  setupSpeakerI2S();

  // WiFi + MQTT
  connectWiFi();
  mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
  mqttClient.setCallback(mqttCallback);
  mqttClient.setBufferSize(2048);  // Larger buffer for audio chunks
  connectMQTT();

  Serial.println("[Ability] Voice Module — Ready!");
  Serial.println("[Info] Speak into the microphone to send a message.");
}

void loop() {
  if (!mqttClient.connected()) connectMQTT();
  mqttClient.loop();

  // TODO: Implement voice activity detection (VAD)
  // When speech detected → stream audio chunks to Hub
  // When silence detected → stop streaming

  captureAndStreamAudio();
}

// ══════════════════════════════════════════════════════════════
// I2S SETUP
// ══════════════════════════════════════════════════════════════

void setupMicI2S() {
  i2s_config_t micConfig = {
    .mode                 = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
    .sample_rate          = SAMPLE_RATE,
    .bits_per_sample      = BITS_PER_SAMPLE,
    .channel_format       = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = I2S_COMM_FORMAT_STAND_I2S,
    .intr_alloc_flags     = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count        = 4,
    .dma_buf_len          = BUFFER_SIZE,
    .use_apll             = false,
    .tx_desc_auto_clear   = false,
    .fixed_mclk           = 0
  };

  i2s_pin_config_t micPins = {
    .bck_io_num   = MIC_I2S_SCK,
    .ws_io_num    = MIC_I2S_WS,
    .data_out_num = I2S_PIN_NO_CHANGE,
    .data_in_num  = MIC_I2S_SD
  };

  i2s_driver_install(MIC_I2S_PORT, &micConfig, 0, NULL);
  i2s_set_pin(MIC_I2S_PORT, &micPins);
  Serial.println("[I2S] Microphone initialized.");
}

void setupSpeakerI2S() {
  // TODO: Configure I2S output for MAX98357A speaker
  Serial.println("[I2S] Speaker initialized.");
}

// ══════════════════════════════════════════════════════════════
// AUDIO CAPTURE & STREAMING
// ══════════════════════════════════════════════════════════════

void captureAndStreamAudio() {
  size_t bytesRead = 0;
  i2s_read(MIC_I2S_PORT, audioBuffer, sizeof(audioBuffer), &bytesRead, portMAX_DELAY);

  if (bytesRead > 0) {
    // TODO: Implement Voice Activity Detection (VAD)
    // Only stream when speech is detected

    // Stream audio chunk to Central Hub via MQTT
    String topic = "ability/local/" + String(MODULE_ID) + "/audio";
    mqttClient.publish(topic.c_str(), (uint8_t*)audioBuffer, bytesRead);
  }
}

// ══════════════════════════════════════════════════════════════
// MQTT — Receiving TTS audio from Hub
// ══════════════════════════════════════════════════════════════

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  // TODO: Receive TTS audio bytes from Hub → play on speaker
  Serial.print("[MQTT] Received ");
  Serial.print(length);
  Serial.println(" bytes of audio from Hub.");
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
