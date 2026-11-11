#ifndef CONFIG_H
#define CONFIG_H

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <LittleFS.h>
#include <Wire.h>
#include <RTClib.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

// ============================================================================
// PIN DEFINITIONS
// ============================================================================
#define PIN_MOTOR_GATE          D5    // GPIO14 - MOSFET Gate
#define PIN_JAM_SENSOR          D6    // GPIO12 - LM393 output
#define PIN_LIGHT_BARRIER       D7    // GPIO13 - IR sensor
#define PIN_HALL_SENSOR         D8    // GPIO15 - Hall sensor
#define PIN_MANUAL_BUTTON       D3    // GPIO0  - Manual button
#define PIN_RTC_SDA             D2    // GPIO4  - I2C SDA
#define PIN_RTC_SCL             D1    // GPIO5  - I2C SCL
#define PIN_POWER_SENSE         A0    // Analog - Power detection
#define PIN_LED_BUILTIN         LED_BUILTIN

// ============================================================================
// MOTOR SETTINGS
// ============================================================================
#define MOTOR_DEFAULT_DURATION_MS   3000
#define MOTOR_MAX_DURATION_MS       30000
#define JAM_CHECK_INTERVAL_MS       100
#define JAM_DEBOUNCE_MS             500

// ============================================================================
// WIFI & MQTT
// ============================================================================
#define WIFI_HOSTNAME           "feedmate"
#define MQTT_PORT               1883
#define MQTT_KEEPALIVE          60

// ============================================================================
// STORAGE
// ============================================================================
#define CONFIG_FILE             "/config.json"
#define SCHEDULE_FILE           "/schedule.json"
#define LOG_FILE                "/events.log"
#define HTML_FILE               "/index.html.gz"
#define MAX_LOG_ENTRIES         100

// ============================================================================
// EVENT TYPES (Muss mit HTML übereinstimmen!)
// ============================================================================
enum EventType {
    EVT_FEEDING_OK = 0,
    EVT_FEEDING_JAM,
    EVT_WATCHDOG_RESET,
    EVT_WIFI_DISCONNECT,
    EVT_WIFI_CONNECT,
    EVT_LOW_BATTERY,
    EVT_POWER_OUTAGE,
    EVT_POWER_RESTORED,
    EVT_MANUAL_FEED,
    EVT_UPDATE_AVAILABLE,
    EVT_DEVICE_SYNC,
    EVT_SENSOR_LOW_FOOD
};

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

struct LogEntry {
    unsigned long timestamp;
    uint8_t eventType;
    char message[64];
};

struct FeedMateConfig {
    // WiFi
    char wifiSsid[33];
    char wifiPassword[65];
    
    // MQTT
    char mqttServer[65];
    uint16_t mqttPort;
    char mqttUser[33];
    char mqttPassword[65];
    char mqttPrefix[33];
    bool mqttEnabled;
    bool haDiscovery;
    
    // Motor & Sensors
    uint16_t jamThreshold;
    uint16_t jamTimeout;
    bool lightBarrierEnabled;
    uint8_t lightBarrierThreshold;
    bool endSwitchEnabled;
    uint8_t rotationsPerPortion;
    uint8_t maxRotations;
    
    // System
    uint16_t deepSleepMinutes;
    char timezone[33];
    char adminPassword[65];  // SHA256 Hash
    
    // SMTP
    char smtpServer[65];
    uint16_t smtpPort;
    char smtpUser[33];
    char smtpPassword[65];
    char smtpFrom[65];
    uint8_t smtpEncryption;  // 0=None, 1=TLS, 2=SSL
    
    // Notifications
    char discordUrl[128];
    char pushoverToken[65];
    char pushoverUser[65];
    char telegramToken[65];
    char telegramChat[65];
    char emailAddr[65];
    bool notifyDiscord[12];
    bool notifyPushover[12];
    bool notifyTelegram[12];
    bool notifyEmail[12];
    
    // Updates
    char updateSource[16];  // github, codeberg, custom
    char customRepoUrl[128];
    bool autoUpdateCheck;
    uint8_t updateInterval;
    bool updateNotify;
    bool autoInstall;
    
    // Logging
    uint8_t logRetention;
    bool logEvents[12];
};

// ============================================================================
// GLOBAL OBJECTS
// ============================================================================
extern RTC_DS3231 rtc;
extern ESP8266WebServer webServer;
extern ESP8266HTTPUpdateServer httpUpdater;
extern WiFiClient wifiClient;
extern PubSubClient mqttClient;
extern WiFiUDP ntpUDP;
extern NTPClient timeClient;

// ============================================================================
// GLOBAL STATE
// ============================================================================
extern FeedMateConfig config;
extern FeedingSlot schedule[8];
extern LogEntry eventLog[MAX_LOG_ENTRIES];
extern uint8_t logCount;
extern uint8_t logIndex;

extern bool motorRunning;
extern unsigned long motorStartTime;
extern unsigned long motorDuration;
extern bool jamActive;
extern unsigned long jamDetectedAt;
extern bool powerOnline;
extern unsigned long lastMqttPublish;
extern unsigned long lastScheduleCheck;
extern uint32_t rotationCount;
extern uint8_t foodLevelPercent;
extern bool isAuthenticated;
extern unsigned long lastAuthTime;

#endif