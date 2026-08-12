#include "config.h"

FeedMateConfig config;
FeedingSlot schedule[8];
LogEntry eventLog[MAX_LOG_ENTRIES];
uint8_t logCount = 0;
uint8_t logIndex = 0;

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

    StaticJsonDocument<4096> doc;
    DeserializationError error = deserializeJson(doc, file);
    file.close();

    if (error) {
        Serial.println("Config parse error, using defaults");
        setDefaultConfig();
        return;
    }

    // WiFi
    strlcpy(config.wifiSsid, doc["wifiSsid"] | "", sizeof(config.wifiSsid));
    strlcpy(config.wifiPassword, doc["wifiPassword"] | "", sizeof(config.wifiPassword));
    
    // MQTT
    strlcpy(config.mqttServer, doc["mqttServer"] | "", sizeof(config.mqttServer));
    config.mqttPort = doc["mqttPort"] | MQTT_PORT;
    strlcpy(config.mqttUser, doc["mqttUser"] | "", sizeof(config.mqttUser));
    strlcpy(config.mqttPassword, doc["mqttPassword"] | "", sizeof(config.mqttPassword));
    strlcpy(config.mqttPrefix, doc["mqttPrefix"] | "feedmate", sizeof(config.mqttPrefix));
    config.mqttEnabled = doc["mqttEnabled"] | false;
    config.haDiscovery = doc["haDiscovery"] | true;
    
    // Motor & Sensors
    config.jamThreshold = doc["jamThreshold"] | 800;
    config.jamTimeout = doc["jamTimeout"] | 500;
    config.lightBarrierEnabled = doc["lightBarrierEnabled"] | false;
    config.lightBarrierThreshold = doc["lightBarrierThreshold"] | 20;
    config.endSwitchEnabled = doc["endSwitchEnabled"] | false;
    config.rotationsPerPortion = doc["rotationsPerPortion"] | 10;
    config.maxRotations = doc["maxRotations"] | 50;
    
    // System
    config.deepSleepMinutes = doc["deepSleepMinutes"] | 1;
    strlcpy(config.timezone, doc["timezone"] | "UTC", sizeof(config.timezone));
    strlcpy(config.adminPassword, doc["adminPassword"] | "", sizeof(config.adminPassword));
    
    // SMTP
    strlcpy(config.smtpServer, doc["smtpServer"] | "", sizeof(config.smtpServer));
    config.smtpPort = doc["smtpPort"] | 587;
    strlcpy(config.smtpUser, doc["smtpUser"] | "", sizeof(config.smtpUser));
    strlcpy(config.smtpPassword, doc["smtpPassword"] | "", sizeof(config.smtpPassword));
    strlcpy(config.smtpFrom, doc["smtpFrom"] | "", sizeof(config.smtpFrom));
    config.smtpEncryption = doc["smtpEncryption"] | 1;
    
    // Notifications
    strlcpy(config.discordUrl, doc["discordUrl"] | "", sizeof(config.discordUrl));
    strlcpy(config.pushoverToken, doc["pushoverToken"] | "", sizeof(config.pushoverToken));
    strlcpy(config.pushoverUser, doc["pushoverUser"] | "", sizeof(config.pushoverUser));
    strlcpy(config.telegramToken, doc["telegramToken"] | "", sizeof(config.telegramToken));
    strlcpy(config.telegramChat, doc["telegramChat"] | "", sizeof(config.telegramChat));
    strlcpy(config.emailAddr, doc["emailAddr"] | "", sizeof(config.emailAddr));
    
    // Updates
    strlcpy(config.updateSource, doc["updateSource"] | "github", sizeof(config.updateSource));
    strlcpy(config.customRepoUrl, doc["customRepoUrl"] | "", sizeof(config.customRepoUrl));
    config.autoUpdateCheck = doc["autoUpdateCheck"] | true;
    config.updateInterval = doc["updateInterval"] | 24;
    config.updateNotify = doc["updateNotify"] | true;
    config.autoInstall = doc["autoInstall"] | false;
    
    // Logging
    config.logRetention = doc["logRetention"] | 7;
    
    Serial.println("Config loaded");
}

void saveConfig() {
    StaticJsonDocument<4096> doc;
    
    // WiFi
    doc["wifiSsid"] = String(config.wifiSsid);
    doc["wifiPassword"] = String(config.wifiPassword);
    
    // MQTT
    doc["mqttServer"] = String(config.mqttServer);
    doc["mqttPort"] = config.mqttPort;
    doc["mqttUser"] = String(config.mqttUser);
    doc["mqttPassword"] = String(config.mqttPassword);
    doc["mqttPrefix"] = String(config.mqttPrefix);
    doc["mqttEnabled"] = config.mqttEnabled;
    doc["haDiscovery"] = config.haDiscovery;
    
    // Motor & Sensors
    doc["jamThreshold"] = config.jamThreshold;
    doc["jamTimeout"] = config.jamTimeout;
    doc["lightBarrierEnabled"] = config.lightBarrierEnabled;
    doc["lightBarrierThreshold"] = config.lightBarrierThreshold;
    doc["endSwitchEnabled"] = config.endSwitchEnabled;
    doc["rotationsPerPortion"] = config.rotationsPerPortion;
    doc["maxRotations"] = config.maxRotations;
    
    // System
    doc["deepSleepMinutes"] = config.deepSleepMinutes;
    doc["timezone"] = String(config.timezone);
    doc["adminPassword"] = String(config.adminPassword);
    
    // SMTP
    doc["smtpServer"] = String(config.smtpServer);
    doc["smtpPort"] = config.smtpPort;
    doc["smtpUser"] = String(config.smtpUser);
    doc["smtpPassword"] = String(config.smtpPassword);
    doc["smtpFrom"] = String(config.smtpFrom);
    doc["smtpEncryption"] = config.smtpEncryption;
    
    // Notifications
    doc["discordUrl"] = String(config.discordUrl);
    doc["pushoverToken"] = String(config.pushoverToken);
    doc["pushoverUser"] = String(config.pushoverUser);
    doc["telegramToken"] = String(config.telegramToken);
    doc["telegramChat"] = String(config.telegramChat);
    doc["emailAddr"] = String(config.emailAddr);
    
    // Updates
    doc["updateSource"] = String(config.updateSource);
    doc["customRepoUrl"] = String(config.customRepoUrl);
    doc["autoUpdateCheck"] = config.autoUpdateCheck;
    doc["updateInterval"] = config.updateInterval;
    doc["updateNotify"] = config.updateNotify;
    doc["autoInstall"] = config.autoInstall;
    
    // Logging
    doc["logRetention"] = config.logRetention;

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

void addLogEntry(uint8_t eventType, const char* message) {
    eventLog[logIndex].timestamp = millis();
    eventLog[logIndex].eventType = eventType;
    strlcpy(eventLog[logIndex].message, message, sizeof(eventLog[logIndex].message));
    
    logIndex = (logIndex + 1) % MAX_LOG_ENTRIES;
    if (logCount < MAX_LOG_ENTRIES) logCount++;
    
    // Save to file periodically
    static unsigned long lastSave = 0;
    if (millis() - lastSave > 60000) {
        saveLogToFile();
        lastSave = millis();
    }
}

void saveLogToFile() {
    File file = LittleFS.open(LOG_FILE, "w");
    if (!file) return;
    
    StaticJsonDocument<8192> doc;
    JsonArray arr = doc.createNestedArray("logs");
    
    for (int i = 0; i < logCount; i++) {
        uint8_t idx = (logIndex + i) % MAX_LOG_ENTRIES;
        JsonObject entry = arr.createNestedObject();
        entry["ts"] = eventLog[idx].timestamp;
        entry["type"] = eventLog[idx].eventType;
        entry["msg"] = eventLog[idx].message;
    }
    
    serializeJson(doc, file);
    file.close();
}

void loadLogFromFile() {
    File file = LittleFS.open(LOG_FILE, "r");
    if (!file) return;
    
    StaticJsonDocument<8192> doc;
    DeserializationError error = deserializeJson(doc, file);
    file.close();
    
    if (error) return;
    
    JsonArray arr = doc["logs"];
    logCount = 0;
    for (int i = 0; i < arr.size() && i < MAX_LOG_ENTRIES; i++) {
        eventLog[logCount].timestamp = arr[i]["ts"];
        eventLog[logCount].eventType = arr[i]["type"];
        strlcpy(eventLog[logCount].message, arr[i]["msg"] | "", sizeof(eventLog[logCount].message));
        logCount++;
    }
    logIndex = 0;
}

void setDefaultConfig() {
    memset(&config, 0, sizeof(config));
    strlcpy(config.mqttPrefix, "feedmate", sizeof(config.mqttPrefix));
    config.mqttPort = MQTT_PORT;
    config.jamThreshold = 800;
    config.jamTimeout = 500;
    config.lightBarrierThreshold = 20;
    config.rotationsPerPortion = 10;
    config.maxRotations = 50;
    config.deepSleepMinutes = 1;
    strlcpy(config.timezone, "UTC", sizeof(config.timezone));
    config.smtpPort = 587;
    config.smtpEncryption = 1;
    strlcpy(config.updateSource, "github", sizeof(config.updateSource));
    config.autoUpdateCheck = true;
    config.updateInterval = 24;
    config.updateNotify = true;
    config.logRetention = 7;
    
    // Default: Enable logging for all events
    for (int i = 0; i < 12; i++) {
        config.logEvents[i] = true;
    }
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