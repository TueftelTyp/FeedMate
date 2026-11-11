#include "config.h"

// Global objects
RTC_DS3231 rtc;
ESP8266WebServer webServer(80);
ESP8266HTTPUpdateServer httpUpdater;
WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 0, 60000);

// Global state
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
bool isAuthenticated = false;
unsigned long lastAuthTime = 0;

// Forward declarations
void setupWiFi();
void setupRTC();
void setupWebServer();
void setupMQTT();
void checkSchedule();
void startMotor(uint16_t durationMs);
void stopMotor();
void checkJamProtection();
void checkSensors();
void publishMQTTStatus();
void handleManualButton();
void checkPowerStatus();

void setup() {
    Serial.begin(115200);
    Serial.println();
    Serial.println("=== FeedMate Booting ===");

    // Initialize pins (Safety First!)
    pinMode(PIN_MOTOR_GATE, OUTPUT);
    digitalWrite(PIN_MOTOR_GATE, LOW);  // CRITICAL: Motor OFF at boot
    pinMode(PIN_JAM_SENSOR, INPUT);
    pinMode(PIN_LIGHT_BARRIER, INPUT);
    pinMode(PIN_HALL_SENSOR, INPUT);
    pinMode(PIN_MANUAL_BUTTON, INPUT_PULLUP);
    pinMode(PIN_LED_BUILTIN, OUTPUT);
    digitalWrite(PIN_LED_BUILTIN, LOW);

    // Initialize Filesystem
    setupLittleFS();
    loadConfig();
    loadSchedule();
    loadLogFromFile();

    // Initialize RTC
    Wire.begin(PIN_RTC_SDA, PIN_RTC_SCL);
    if (!rtc.begin()) {
        Serial.println("ERROR: RTC not found!");
    } else if (rtc.lostPower()) {
        Serial.println("RTC lost power, setting to compile time");
        rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    }

    // Initialize NTP
    timeClient.begin();

    // Connect WiFi
    setupWiFi();

    // Start Web Server
    setupWebServer();

    // Start MQTT (if enabled)
    if (config.mqttEnabled) {
        setupMQTT();
    }

    // Check Power Status
    checkPowerStatus();

    // Log boot event
    addLogEntry(EVT_WATCHDOG_RESET, "FeedMate boot completed");

    Serial.println("=== FeedMate Ready ===");
}

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
            addLogEntry(EVT_FEEDING_OK, "Feeding completed successfully");
            Serial.println("Feeding completed");
        }
    }

    // Sensor updates
    checkSensors();

    // Manual button
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

    // Auto-check for updates
    static unsigned long lastUpdateCheck = 0;
    if (config.autoUpdateCheck && millis() - lastUpdateCheck >= (config.updateInterval * 3600000UL)) {
        lastUpdateCheck = millis();
        // checkForUpdates(); // Implement later
    }
}

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
            addLogEntry(EVT_WIFI_CONNECT, "WiFi connected successfully");
        } else {
            Serial.println();
            Serial.println("WiFi failed, starting AP mode");
            startFallbackAP();
            addLogEntry(EVT_WIFI_DISCONNECT, "WiFi failed, AP mode started");
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

void checkPowerStatus() {
    int raw = analogRead(PIN_POWER_SENSE);
    bool nowOnline = raw > 500;  // Threshold ~2.5V at A0

    if (nowOnline != powerOnline) {
        powerOnline = nowOnline;
        Serial.printf("Power status: %s\n", powerOnline ? "ONLINE" : "BATTERY");
        
        if (powerOnline) {
            addLogEntry(EVT_POWER_RESTORED, "Mains power restored");
        } else {
            addLogEntry(EVT_POWER_OUTAGE, "Power outage detected");
            // Enter deep sleep if configured
            if (config.deepSleepMinutes > 0) {
                Serial.println("Entering deep sleep...");
                ESP.deepSleep(config.deepSleepMinutes * 60000000);
            }
        }

        if (config.mqttEnabled && mqttClient.connected()) {
            mqttClient.publish(
                (String(config.mqttPrefix) + "/status/power").c_str(),
                powerOnline ? "1" : "0", true);
        }
    }
}