/*
 * ============================================================================
 * FeedMate - Smart Cat Feeder Firmware
 * ============================================================================
 * Hardware: Wemos D1 Mini (ESP8266)
 * Features:
 *   - RTC-based feeding schedule (DS3231)
 *   - Motor control with fail-safe (MOSFET + pull-down)
 *   - Hardware jam protection (LM393 comparator)
 *   - Optional light barrier & hall sensor
 *   - Web interface for configuration
 *   - MQTT integration (Home Assistant / ESPHome)
 *   - Deep sleep with UPS (18650 batteries)
 *   - OTA update support
 * ============================================================================
 */

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <LittleFS.h>
#include <Wire.h>
#include <RTClib.h>

// ============================================================================
// CONFIGURATION
// ============================================================================

// --- Pin Definitions ---
#define PIN_MOTOR_GATE      D5   // GPIO14 - MOSFET Gate (with 10k pull-down!)
#define PIN_JAM_SENSOR      D6   // GPIO12 - LM393 output (jam detection)
#define PIN_LIGHT_BARRIER   D7   // GPIO13 - IR light barrier (food level)
#define PIN_HALL_SENSOR     D8   // GPIO15 - Hall sensor (rotations)
#define PIN_MANUAL_BUTTON   D3   // GPIO0  - Manual feeding button
#define PIN_RTC_SDA         D2   // GPIO4  - I2C SDA
#define PIN_RTC_SCL         D1   // GPIO5  - I2C SCL
#define PIN_POWER_SENSE     A0   // Analog - Power supply detection (voltage divider)

// --- Motor Settings ---
#define MOTOR_DEFAULT_DURATION_MS   3000    // Default feeding duration
#define MOTOR_MAX_DURATION_MS       30000   // Safety limit
#define JAM_CHECK_INTERVAL_MS       100     // How often to check jam sensor

// --- Jam Protection ---
#define JAM_DEBOUNCE_MS             500     // Jam must persist this long
#define JAM_LED_PIN                 LED_BUILTIN  // Use built-in LED as jam indicator

// --- WiFi & MQTT ---
#define WIFI_HOSTNAME               "feedmate"
#define MQTT_PORT                   1883
#define MQTT_KEEPALIVE              60

// --- Storage Keys ---
#define CONFIG_FILE                 "/config.json"
#define SCHEDULE_FILE               "/schedule.json"

// ============================================================================
// GLOBAL OBJECTS
// ============================================================================

RTC_DS3231 rtc;
ESP8266WebServer webServer(80);
ESP8266HTTPUpdateServer httpUpdater;
WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

// ============================================================================
// DATA STRUCTURES
// ============================================================================

struct FeedingSlot {
  bool active;
  uint8_t hour;
  uint8_t minute;
  uint16_t durationMs;
  bool lastRunToday;
};

struct FeedMateConfig {
  char wifiSsid[33];
  char wifiPassword[65];
  char mqttServer[65];
  uint16_t mqttPort;
  char mqttUser[33];
  char mqttPassword[65];
  char mqttPrefix[33];
  bool mqttEnabled;
  bool haDiscovery;
  uint16_t jamThreshold;      // mA
  uint16_t jamTimeout;        // ms
  uint16_t deepSleepMinutes;
  char timezone[33];
  bool lightBarrierEnabled;
  uint8_t lightBarrierThreshold;
  bool endSwitchEnabled;
  uint8_t rotationsPerPortion;
  uint8_t maxRotations;
};

// ============================================================================
// GLOBAL STATE
// ============================================================================

FeedMateConfig config;
FeedingSlot schedule[8];

bool motorRunning = false;
unsigned long motorStartTime = 0;
unsigned long motorDuration = 0;
bool jamActive = false;
unsigned long jamDetectedAt = 0;
bool powerOnline = true;
unsigned long lastMqttPublish = 0;
unsigned long lastScheduleCheck = 0;
uint32_t rotationCount = 0;
uint8_t foodLevelPercent = 100;

// ============================================================================
// FORWARD DECLARATIONS
// ============================================================================

void setupWiFi();
void setupRTC();
void setupWebServer();
void setupMQTT();
void setupLittleFS();
void loadConfig();
void saveConfig();
void loadSchedule();
void saveSchedule();
void checkSchedule();
void startMotor(uint16_t durationMs);
void stopMotor();
void checkJamProtection();
void checkSensors();
void publishMQTTStatus();
void handleManualButton();
String getWebPage();

// ============================================================================
// SETUP
// ============================================================================

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println("=== FeedMate Booting ===");

  // --- Initialize Pins (Safety First!) ---
  pinMode(PIN_MOTOR_GATE, OUTPUT);
  digitalWrite(PIN_MOTOR_GATE, LOW);  // CRITICAL: Motor OFF at boot
  pinMode(PIN_JAM_SENSOR, INPUT);
  pinMode(PIN_LIGHT_BARRIER, INPUT);
  pinMode(PIN_HALL_SENSOR, INPUT);
  pinMode(PIN_MANUAL_BUTTON, INPUT_PULLUP);
  pinMode(JAM_LED_PIN, OUTPUT);
  digitalWrite(JAM_LED_PIN, LOW);

  // --- Load Filesystem ---
  setupLittleFS();
  loadConfig();
  loadSchedule();

  // --- Initialize RTC ---
  setupRTC();

  // --- Connect WiFi ---
  setupWiFi();

  // --- Start Web Server ---
  setupWebServer();

  // --- Start MQTT (if enabled) ---
  if (config.mqttEnabled) {
    setupMQTT();
  }

  // --- Check Power Status ---
  checkPowerStatus();

  Serial.println("=== FeedMate Ready ===");
}

// ============================================================================
// MAIN LOOP
// ============================================================================

void loop() {
  webServer.handleClient();

  if (config.mqttEnabled && mqttClient.connected()) {
    mqttClient.loop();
  }

  // Check schedule every second
  if (millis() - lastScheduleCheck >= 1000) {
    lastScheduleCheck = millis();
    checkSchedule();
  }

  // Motor runtime check
  if (motorRunning) {
    checkJamProtection();
    if (millis() - motorStartTime >= motorDuration) {
      stopMotor();
      Serial.println("Feeding completed");
    }
  }

  // Sensor updates (light barrier, hall sensor)
  checkSensors();

  // Manual button (with debounce)
  handleManualButton();

  // MQTT status publish (every 30 seconds)
  if (config.mqttEnabled && millis() - lastMqttPublish >= 30000) {
    lastMqttPublish = millis();
    publishMQTTStatus();
  }

  // Power status check (every 5 seconds)
  static unsigned long lastPowerCheck = 0;
  if (millis() - lastPowerCheck >= 5000) {
    lastPowerCheck = millis();
    checkPowerStatus();
  }
}

// ============================================================================
// WIFI SETUP
// ============================================================================

void setupWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.hostname(WIFI_HOSTNAME);

  if (strlen(config.wifiSsid) > 0) {
    WiFi.begin(config.wifiSsid, config.wifiPassword);
    Serial.print("Connecting to WiFi");

    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 30) {
      delay(500);
      Serial.print(".");
      attempts++;
    }

    if (WiFi.status() == WL_CONNECTED) {
      Serial.println();
      Serial.print("Connected! IP: ");
      Serial.println(WiFi.localIP());
    } else {
      Serial.println();
      Serial.println("WiFi failed, starting AP mode");
      startFallbackAP();
    }
  } else {
    Serial.println("No WiFi configured, starting AP mode");
    startFallbackAP();
  }
}

void startFallbackAP() {
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP("FeedMate-Setup", "feedmate123");
  Serial.print("AP IP: ");
  Serial.println(WiFi.softAPIP());
}

// ============================================================================
// RTC SETUP
// ============================================================================

void setupRTC() {
  Wire.begin(PIN_RTC_SDA, PIN_RTC_SCL);

  if (!rtc.begin()) {
    Serial.println("ERROR: RTC not found!");
    return;
  }

  if (rtc.lostPower()) {
    Serial.println("RTC lost power, setting to compile time");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  DateTime now = rtc.now();
  Serial.printf("RTC Time: %04d-%02d-%02d %02d:%02d:%02d\n",
    now.year(), now.month(), now.day(),
    now.hour(), now.minute(), now.second());
}

// ============================================================================
// WEB SERVER
// ============================================================================

void setupWebServer() {
  // Serve the main web interface
  webServer.on("/", HTTP_GET, []() {
    webServer.send(200, "text/html", getWebPage());
  });

  // API: Get current status
  webServer.on("/api/status", HTTP_GET, []() {
    StaticJsonDocument<512> doc;
    doc["motorRunning"] = motorRunning;
    doc["jamActive"] = jamActive;
    doc["powerOnline"] = powerOnline;
    doc["foodLevel"] = foodLevelPercent;
    doc["rotations"] = rotationCount;

    DateTime now = rtc.now();
    char timeBuf[20];
    sprintf(timeBuf, "%04d-%02d-%02d %02d:%02d:%02d",
      now.year(), now.month(), now.day(),
      now.hour(), now.minute(), now.second());
    doc["rtcTime"] = timeBuf;

    String response;
    serializeJson(doc, response);
    webServer.send(200, "application/json", response);
  });

  // API: Start manual feeding
  webServer.on("/api/feed", HTTP_POST, []() {
    uint16_t duration = MOTOR_DEFAULT_DURATION_MS;
    if (webServer.hasArg("duration")) {
      duration = webServer.arg("duration").toInt();
      if (duration > MOTOR_MAX_DURATION_MS) duration = MOTOR_MAX_DURATION_MS;
    }

    if (!motorRunning && !jamActive) {
      startMotor(duration);
      webServer.send(200, "application/json", "{\"status\":\"started\"}");
    } else {
      webServer.send(409, "application/json", "{\"error\":\"Motor busy or jam active\"}");
    }
  });

  // API: Get/Set schedule
  webServer.on("/api/schedule", HTTP_GET, []() {
    StaticJsonDocument<1024> doc;
    JsonArray arr = doc.createNestedArray("slots");
    for (int i = 0; i < 8; i++) {
      JsonObject slot = arr.createNestedObject();
      slot["active"] = schedule[i].active;
      slot["hour"] = schedule[i].hour;
      slot["minute"] = schedule[i].minute;
      slot["duration"] = schedule[i].durationMs;
    }
    String response;
    serializeJson(doc, response);
    webServer.send(200, "application/json", response);
  });

  webServer.on("/api/schedule", HTTP_POST, []() {
    String body = webServer.arg("plain");
    StaticJsonDocument<1024> doc;
    DeserializationError error = deserializeJson(doc, body);

    if (!error) {
      JsonArray arr = doc["slots"];
      for (int i = 0; i < 8 && i < arr.size(); i++) {
        schedule[i].active = arr[i]["active"];
        schedule[i].hour = arr[i]["hour"];
        schedule[i].minute = arr[i]["minute"];
        schedule[i].durationMs = arr[i]["duration"];
        schedule[i].lastRunToday = false;
      }
      saveSchedule();
      webServer.send(200, "application/json", "{\"status\":\"saved\"}");
    } else {
      webServer.send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
    }
  });

  // API: Get/Set config
  webServer.on("/api/config", HTTP_GET, []() {
    StaticJsonDocument<1024> doc;
    doc["wifiSsid"] = String(config.wifiSsid);
    doc["mqttServer"] = String(config.mqttServer);
    doc["mqttPort"] = config.mqttPort;
    doc["mqttEnabled"] = config.mqttEnabled;
    doc["jamThreshold"] = config.jamThreshold;
    doc["lightBarrierEnabled"] = config.lightBarrierEnabled;
    doc["endSwitchEnabled"] = config.endSwitchEnabled;

    String response;
    serializeJson(doc, response);
    webServer.send(200, "application/json", response);
  });

  // OTA update endpoint
  httpUpdater.setup(&webServer, "/update", "admin", "feedmate");

  webServer.begin();
  Serial.println("Web server started");
}

// ============================================================================
// MQTT SETUP
// ============================================================================

void setupMQTT() {
  if (strlen(config.mqttServer) == 0) return;

  mqttClient.setServer(config.mqttServer, config.mqttPort);
  mqttClient.setCallback(mqttCallback);

  connectMQTT();
}

void connectMQTT() {
  String clientId = String(WIFI_HOSTNAME) + "-" + WiFi.macAddress();
  bool connected = false;

  if (strlen(config.mqttUser) > 0) {
    connected = mqttClient.connect(
      clientId.c_str(),
      config.mqttUser,
      config.mqttPassword,
      String(config.mqttPrefix) + "/status/online".c_str(),
      1, true, "0"
    );
  } else {
    connected = mqttClient.connect(
      clientId.c_str(),
      String(config.mqttPrefix) + "/status/online".c_str(),
      1, true, "0"
    );
  }

  if (connected) {
    Serial.println("MQTT connected");
    mqttClient.publish(
      (String(config.mqttPrefix) + "/status/online").c_str(),
      "1", true);

    // Subscribe to control topics
    mqttClient.subscribe(
      (String(config.mqttPrefix) + "/control/feed_now").c_str());
    mqttClient.subscribe(
      (String(config.mqttPrefix) + "/control/feed_duration").c_str());
    mqttClient.subscribe(
      (String(config.mqttPrefix) + "/control/reset_sensors").c_str());
  } else {
    Serial.print("MQTT failed, rc=");
    Serial.println(mqttClient.state());
  }
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  String topicStr = String(topic);
  String payloadStr = "";
  for (unsigned int i = 0; i < length; i++) {
    payloadStr += (char)payload[i];
  }

  Serial.printf("MQTT: %s = %s\n", topic, payload);

  if (topicStr.endsWith("/control/feed_now")) {
    if (payloadStr == "1" || payloadStr == "true") {
      if (!motorRunning && !jamActive) {
        startMotor(MOTOR_DEFAULT_DURATION_MS);
      }
    }
  }
  else if (topicStr.endsWith("/control/feed_duration")) {
    uint16_t duration = payloadStr.toInt();
    if (duration > 0 && duration <= MOTOR_MAX_DURATION_MS) {
      if (!motorRunning && !jamActive) {
        startMotor(duration);
      }
    }
  }
  else if (topicStr.endsWith("/control/reset_sensors")) {
    jamActive = false;
    rotationCount = 0;
    foodLevelPercent = 100;
    digitalWrite(JAM_LED_PIN, LOW);
    Serial.println("Sensors reset via MQTT");
  }
}

// ============================================================================
// MOTOR CONTROL
// ============================================================================

void startMotor(uint16_t durationMs) {
  if (jamActive) {
    Serial.println("Motor start blocked: jam active");
    return;
  }

  if (durationMs > MOTOR_MAX_DURATION_MS) {
    durationMs = MOTOR_MAX_DURATION_MS;
  }

  motorRunning = true;
  motorStartTime = millis();
  motorDuration = durationMs;

  digitalWrite(PIN_MOTOR_GATE, HIGH);
  Serial.printf("Motor started for %d ms\n", durationMs);

  // Publish MQTT event
  if (config.mqttEnabled && mqttClient.connected()) {
    mqttClient.publish(
      (String(config.mqttPrefix) + "/events/last").c_str(),
      "feeding_started");
  }
}

void stopMotor() {
  motorRunning = false;
  digitalWrite(PIN_MOTOR_GATE, LOW);
  Serial.println("Motor stopped");

  if (config.mqttEnabled && mqttClient.connected()) {
    mqttClient.publish(
      (String(config.mqttPrefix) + "/events/last").c_str(),
      "feeding_completed");
  }
}

// ============================================================================
// JAM PROTECTION
// ============================================================================

void checkJamProtection() {
  // LM393 output: HIGH = jam detected (current above threshold)
  bool jamSignal = digitalRead(PIN_JAM_SENSOR);

  if (jamSignal) {
    if (jamDetectedAt == 0) {
      jamDetectedAt = millis();
    }
    else if (millis() - jamDetectedAt >= JAM_DEBOUNCE_MS) {
      // Jam confirmed!
      if (!jamActive) {
        jamActive = true;
        stopMotor();
        digitalWrite(JAM_LED_PIN, HIGH);
        Serial.println("JAM DETECTED - Motor stopped!");

        if (config.mqttEnabled && mqttClient.connected()) {
          mqttClient.publish(
            (String(config.mqttPrefix) + "/status/jam_active").c_str(),
            "1", true);
          mqttClient.publish(
            (String(config.mqttPrefix) + "/events/jam").c_str(),
            "Jam protection triggered");
        }
      }
    }
  } else {
    jamDetectedAt = 0;
  }
}

// ============================================================================
// SENSOR CHECKS
// ============================================================================

void checkSensors() {
  // Light barrier (food level)
  if (config.lightBarrierEnabled) {
    // Assuming sensor outputs HIGH when food present, LOW when empty
    bool foodPresent = digitalRead(PIN_LIGHT_BARRIER);

    if (!foodPresent) {
      foodLevelPercent = max(0, foodLevelPercent - 1);
    } else {
      foodLevelPercent = min(100, foodLevelPercent + 2);
    }

    // Alert if below threshold
    if (foodLevelPercent <= config.lightBarrierThreshold) {
      static bool alertSent = false;
      if (!alertSent) {
        alertSent = true;
        Serial.println("WARNING: Food level low!");
        if (config.mqttEnabled && mqttClient.connected()) {
          mqttClient.publish(
            (String(config.mqttPrefix) + "/sensor/food_level").c_str(),
            String(foodLevelPercent).c_str());
        }
      }
    } else {
      // Reset would be handled by manual reset button
    }
  }

  // Hall sensor (rotation counting)
  if (config.endSwitchEnabled) {
    static bool lastHallState = HIGH;
    bool hallState = digitalRead(PIN_HALL_SENSOR);

    // Count falling edges (magnet passing)
    if (lastHallState == HIGH && hallState == LOW) {
      rotationCount++;

      // Safety: stop if too many rotations
      if (rotationCount >= config.maxRotations && motorRunning) {
        Serial.println("Max rotations reached - stopping motor");
        stopMotor();
      }
    }
    lastHallState = hallState;
  }
}

// ============================================================================
// SCHEDULE CHECK
// ============================================================================

void checkSchedule() {
  DateTime now = rtc.now();
  uint8_t currentHour = now.hour();
  uint8_t currentMinute = now.minute();

  // Reset daily flags at midnight
  static uint8_t lastDay = 0;
  if (now.day() != lastDay) {
    for (int i = 0; i < 8; i++) {
      schedule[i].lastRunToday = false;
    }
    lastDay = now.day();
  }

  for (int i = 0; i < 8; i++) {
    if (!schedule[i].active) continue;
    if (schedule[i].lastRunToday) continue;

    if (currentHour == schedule[i].hour && currentMinute == schedule[i].minute) {
      Serial.printf("Scheduled feeding #%d at %02d:%02d\n", i, currentHour, currentMinute);
      startMotor(schedule[i].durationMs);
      schedule[i].lastRunToday = true;
      saveSchedule();
    }
  }
}

// ============================================================================
// MANUAL BUTTON
// ============================================================================

void handleManualButton() {
  static unsigned long lastButtonPress = 0;
  static bool lastButtonState = HIGH;

  bool buttonState = digitalRead(PIN_MANUAL_BUTTON);

  // Detect falling edge (button pressed, active LOW)
  if (lastButtonState == HIGH && buttonState == LOW) {
    if (millis() - lastButtonPress > 500) {  // 500ms debounce
      lastButtonPress = millis();

      if (jamActive) {
        // Reset jam
        jamActive = false;
        digitalWrite(JAM_LED_PIN, LOW);
        Serial.println("Jam reset via manual button");

        // Try feeding after reset
        delay(500);
        if (!motorRunning) {
          startMotor(MOTOR_DEFAULT_DURATION_MS);
        }
      } else if (!motorRunning) {
        startMotor(MOTOR_DEFAULT_DURATION_MS);
      }
    }
  }
  lastButtonState = buttonState;
}

// ============================================================================
// POWER STATUS
// ============================================================================

void checkPowerStatus() {
  // A0 reads voltage from divider (5V -> ~1V at A0)
  int raw = analogRead(PIN_POWER_SENSE);
  bool nowOnline = raw > 500;  // Threshold ~2.5V at A0 = ~12.5V at divider input

  if (nowOnline != powerOnline) {
    powerOnline = nowOnline;
    Serial.printf("Power status: %s\n", powerOnline ? "ONLINE" : "BATTERY");

    if (config.mqttEnabled && mqttClient.connected()) {
      mqttClient.publish(
        (String(config.mqttPrefix) + "/status/power").c_str(),
        powerOnline ? "1" : "0", true);
    }
  }
}

// ============================================================================
// MQTT STATUS PUBLISH
// ============================================================================

void publishMQTTStatus() {
  if (!mqttClient.connected()) {
    connectMQTT();
    return;
  }

  String prefix = String(config.mqttPrefix);

  mqttClient.publish((prefix + "/status/online").c_str(), "1", true);
  mqttClient.publish((prefix + "/status/motor_running").c_str(),
    motorRunning ? "1" : "0");
  mqttClient.publish((prefix + "/status/jam_active").c_str(),
    jamActive ? "1" : "0", true);

  if (config.lightBarrierEnabled) {
    mqttClient.publish((prefix + "/sensor/food_level").c_str(),
      String(foodLevelPercent).c_str());
  }

  if (config.endSwitchEnabled) {
    mqttClient.publish((prefix + "/sensor/rotations").c_str(),
      String(rotationCount).c_str());
  }

  // WiFi signal
  mqttClient.publish((prefix + "/status/wifi_rssi").c_str(),
    String(WiFi.RSSI()).c_str());
}

// ============================================================================
// FILESYSTEM & STORAGE
// ============================================================================

void setupLittleFS() {
  if (!LittleFS.begin()) {
    Serial.println("LittleFS mount failed, formatting...");
    LittleFS.format();
    LittleFS.begin();
  }
  Serial.println("LittleFS ready");
}

void loadConfig() {
  File file = LittleFS.open(CONFIG_FILE, "r");
  if (!file) {
    Serial.println("No config file, using defaults");
    setDefaultConfig();
    return;
  }

  StaticJsonDocument<2048> doc;
  DeserializationError error = deserializeJson(doc, file);
  file.close();

  if (error) {
    Serial.println("Config parse error, using defaults");
    setDefaultConfig();
    return;
  }

  strlcpy(config.wifiSsid, doc["wifiSsid"] | "", sizeof(config.wifiSsid));
  strlcpy(config.wifiPassword, doc["wifiPassword"] | "", sizeof(config.wifiPassword));
  strlcpy(config.mqttServer, doc["mqttServer"] | "", sizeof(config.mqttServer));
  config.mqttPort = doc["mqttPort"] | MQTT_PORT;
  strlcpy(config.mqttUser, doc["mqttUser"] | "", sizeof(config.mqttUser));
  strlcpy(config.mqttPassword, doc["mqttPassword"] | "", sizeof(config.mqttPassword));
  strlcpy(config.mqttPrefix, doc["mqttPrefix"] | "feedmate", sizeof(config.mqttPrefix));
  config.mqttEnabled = doc["mqttEnabled"] | false;
  config.haDiscovery = doc["haDiscovery"] | true;
  config.jamThreshold = doc["jamThreshold"] | 800;
  config.jamTimeout = doc["jamTimeout"] | 500;
  config.deepSleepMinutes = doc["deepSleepMinutes"] | 1;
  strlcpy(config.timezone, doc["timezone"] | "UTC", sizeof(config.timezone));
  config.lightBarrierEnabled = doc["lightBarrierEnabled"] | false;
  config.lightBarrierThreshold = doc["lightBarrierThreshold"] | 20;
  config.endSwitchEnabled = doc["endSwitchEnabled"] | false;
  config.rotationsPerPortion = doc["rotationsPerPortion"] | 10;
  config.maxRotations = doc["maxRotations"] | 50;

  Serial.println("Config loaded");
}

void saveConfig() {
  StaticJsonDocument<2048> doc;
  doc["wifiSsid"] = String(config.wifiSsid);
  doc["wifiPassword"] = String(config.wifiPassword);
  doc["mqttServer"] = String(config.mqttServer);
  doc["mqttPort"] = config.mqttPort;
  doc["mqttUser"] = String(config.mqttUser);
  doc["mqttPassword"] = String(config.mqttPassword);
  doc["mqttPrefix"] = String(config.mqttPrefix);
  doc["mqttEnabled"] = config.mqttEnabled;
  doc["haDiscovery"] = config.haDiscovery;
  doc["jamThreshold"] = config.jamThreshold;
  doc["jamTimeout"] = config.jamTimeout;
  doc["deepSleepMinutes"] = config.deepSleepMinutes;
  doc["timezone"] = String(config.timezone);
  doc["lightBarrierEnabled"] = config.lightBarrierEnabled;
  doc["lightBarrierThreshold"] = config.lightBarrierThreshold;
  doc["endSwitchEnabled"] = config.endSwitchEnabled;
  doc["rotationsPerPortion"] = config.rotationsPerPortion;
  doc["maxRotations"] = config.maxRotations;

  File file = LittleFS.open(CONFIG_FILE, "w");
  if (file) {
    serializeJson(doc, file);
    file.close();
    Serial.println("Config saved");
  }
}

void loadSchedule() {
  File file = LittleFS.open(SCHEDULE_FILE, "r");
  if (!file) {
    Serial.println("No schedule file, using defaults");
    setDefaultSchedule();
    return;
  }

  StaticJsonDocument<2048> doc;
  DeserializationError error = deserializeJson(doc, file);
  file.close();

  if (error) {
    setDefaultSchedule();
    return;
  }

  JsonArray arr = doc["slots"];
  for (int i = 0; i < 8 && i < arr.size(); i++) {
    schedule[i].active = arr[i]["active"] | false;
    schedule[i].hour = arr[i]["hour"] | 0;
    schedule[i].minute = arr[i]["minute"] | 0;
    schedule[i].durationMs = arr[i]["duration"] | MOTOR_DEFAULT_DURATION_MS;
    schedule[i].lastRunToday = false;
  }

  Serial.println("Schedule loaded");
}

void saveSchedule() {
  StaticJsonDocument<2048> doc;
  JsonArray arr = doc.createNestedArray("slots");

  for (int i = 0; i < 8; i++) {
    JsonObject slot = arr.createNestedObject();
    slot["active"] = schedule[i].active;
    slot["hour"] = schedule[i].hour;
    slot["minute"] = schedule[i].minute;
    slot["duration"] = schedule[i].durationMs;
  }

  File file = LittleFS.open(SCHEDULE_FILE, "w");
  if (file) {
    serializeJson(doc, file);
    file.close();
    Serial.println("Schedule saved");
  }
}

void setDefaultConfig() {
  strlcpy(config.wifiSsid, "", sizeof(config.wifiSsid));
  strlcpy(config.wifiPassword, "", sizeof(config.wifiPassword));
  strlcpy(config.mqttServer, "", sizeof(config.mqttServer));
  config.mqttPort = MQTT_PORT;
  strlcpy(config.mqttPrefix, "feedmate", sizeof(config.mqttPrefix));
  config.mqttEnabled = false;
  config.haDiscovery = true;
  config.jamThreshold = 800;
  config.jamTimeout = 500;
  config.deepSleepMinutes = 1;
  strlcpy(config.timezone, "UTC", sizeof(config.timezone));
  config.lightBarrierEnabled = false;
  config.lightBarrierThreshold = 20;
  config.endSwitchEnabled = false;
  config.rotationsPerPortion = 10;
  config.maxRotations = 50;
}

void setDefaultSchedule() {
  for (int i = 0; i < 8; i++) {
    schedule[i].active = false;
    schedule[i].hour = 0;
    schedule[i].minute = 0;
    schedule[i].durationMs = MOTOR_DEFAULT_DURATION_MS;
    schedule[i].lastRunToday = false;
  }
  // Default: 3 feedings per day
  schedule[0].active = true; schedule[0].hour = 7;  schedule[0].minute = 0;
  schedule[1].active = true; schedule[1].hour = 12; schedule[1].minute = 0;
  schedule[2].active = true; schedule[2].hour = 18; schedule[2].minute = 0;
}

// ============================================================================
// WEB INTERFACE (Minimal placeholder)
// ============================================================================

String getWebPage() {
  // In production, load this from LittleFS (/index.html.gz)
  // For now, return a minimal status page
  String html = "<!DOCTYPE html><html><head><title>FeedMate</title>";
  html += "<meta charset='UTF-8'>";
  html += "<style>body{font-family:sans-serif;margin:40px;}";
  html += ".status{padding:20px;background:#f0f0f0;border-radius:8px;margin:10px 0;}</style>";
  html += "</head><body>";
  html += "<h1>FeedMate Status</h1>";

  html += "<div class='status'>";
  html += "<h2>Motor</h2>";
  html += motorRunning ? "<p style='color:green'>RUNNING</p>" : "<p>IDLE</p>";
  html += "</div>";

  html += "<div class='status'>";
  html += "<h2>Jam Protection</h2>";
  html += jamActive ? "<p style='color:red'>ACTIVE</p>" : "<p>OK</p>";
  html += "</div>";

  html += "<div class='status'>";
  html += "<h2>Power</h2>";
  html += powerOnline ? "<p>ONLINE (5V PSU)</p>" : "<p>BATTERY MODE</p>";
  html += "</div>";

  if (config.lightBarrierEnabled) {
    html += "<div class='status'>";
    html += "<h2>Food Level</h2>";
    html += "<p>" + String(foodLevelPercent) + "%</p>";
    html += "</div>";
  }

  html += "<div class='status'>";
  html += "<h2>RTC Time</h2>";
  DateTime now = rtc.now();
  char buf[20];
  sprintf(buf, "%04d-%02d-%02d %02d:%02d:%02d",
    now.year(), now.month(), now.day(),
    now.hour(), now.minute(), now.second());
  html += "<p>" + String(buf) + "</p>";
  html += "</div>";

  html += "<h2>Quick Actions</h2>";
  html += "<button onclick=\"fetch('/api/feed',{method:'POST'})\">Feed Now (3s)</button>";
  html += " <button onclick=\"location.href='/update'\">OTA Update</button>";

  html += "</body></html>";
  return html;
}