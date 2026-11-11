#include "config.h"

FeedMateConfig config;
FeedingSlot schedule[MAX_SCHEDULE_ENTRIES];
LogEntry eventLog[MAX_LOG_ENTRIES];
uint8_t logCount = 0;
uint8_t logIndex = 0;

bool storageReady = false;
bool logsDirty = false;
class ChecksumPrint : public Print {
public:
    uint32_t hash = 2166136261UL;
    size_t write(uint8_t b) override { hash = (hash ^ b) * 16777619UL; return 1; }
};
void setupLittleFS() {
#if defined(ESP32)
    storageReady = LittleFS.begin(false); // Never format on mount failure.
#else
    LittleFSConfig options;
    options.setAutoFormat(false);
    LittleFS.setConfig(options);
    storageReady = LittleFS.begin();
#endif
    if (!storageReady) Serial.println("Storage unavailable: motor disabled; no automatic format");
}

bool atomicJson(const char* path, const JsonDocument& doc) {
    if (!storageReady || doc.overflowed()) return false;
    String temp = String(path) + ".tmp";
    File file = LittleFS.open(temp, "w");
    if (!file) return false;
    size_t expected = measureJson(doc);
    size_t written = serializeJson(doc, file);
    file.flush();
    bool valid = written == expected && file.size() == expected;
    file.close();
    // Read back before replacing the last committed file.
    File verify = LittleFS.open(temp, "r");
    ChecksumPrint expectedHash, actualHash;
    serializeJson(doc, expectedHash);
    if (!verify) valid = false;
    while (verify && verify.available()) actualHash.write(uint8_t(verify.read()));
    valid = valid && expectedHash.hash == actualHash.hash;
    verify.close();
    if (!valid || !LittleFS.rename(temp, path)) {
        Serial.printf("Storage commit failed: %s\n", path);
        return false;
    }
    return true;
}

void loadConfig() {
    setDefaultConfig();
    File file = LittleFS.open(CONFIG_FILE, "r");
    if (!file) {
        Serial.println("No config file, using defaults");
        setDefaultConfig();
        return;
    }

    DynamicJsonDocument doc(8192);
    DeserializationError error = deserializeJson(doc, file);
    file.close();

    if (error || !doc.is<JsonObject>()) {
        Serial.println("Config parse error: feeding disabled until recovery");
        setDefaultConfig();
        storageReady = false;
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
    config.deepSleepMinutes = doc["deepSleepMinutes"] | 0;
    strlcpy(config.timezone, doc["timezone"] | "UTC", sizeof(config.timezone));
    strlcpy(config.adminPassword, doc["adminPassword"] | "", sizeof(config.adminPassword));
    strlcpy(config.deviceName, doc["deviceName"] | "FeedMate", sizeof(config.deviceName));
    strlcpy(config.fleetKeyHash, doc["fleetKeyHash"] | "", sizeof(config.fleetKeyHash));
    
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
    
    if (!config.timezone[0] || config.jamTimeout<50 || config.jamTimeout>1000 ||
        !config.rotationsPerPortion || !config.maxRotations ||
        config.rotationsPerPortion>config.maxRotations) {
        storageReady=false;
        Serial.println("Unsafe persisted configuration: feeding disabled");
    }
    Serial.println("Config loaded");
}

bool saveConfig() {
    DynamicJsonDocument doc(8192);
    
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
    doc["deviceName"] = String(config.deviceName);
    doc["fleetKeyHash"] = String(config.fleetKeyHash);
    
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

    return atomicJson(CONFIG_FILE, doc);
}

void loadSchedule() {
    setDefaultSchedule();
    if (!LittleFS.exists(SCHEDULE_FILE)) {
        if (!saveSchedule()) storageReady = false;
        return;
    }
    File f = LittleFS.open(SCHEDULE_FILE, "r");
    DynamicJsonDocument doc(12288);
    if (!f || deserializeJson(doc, f) || !doc["slots"].is<JsonArray>() || doc["slots"].size() > MAX_SCHEDULE_ENTRIES) {
        storageReady = false; return;
    }
    JsonArray slots=doc["slots"].as<JsonArray>();
    for (size_t i=0;i<slots.size();i++) {
        JsonObject slot = doc["slots"][i];
        long hour=slot["hour"] | -1, minute=slot["minute"] | -1, second=slot["second"] | 0, duration=slot["duration"] | MOTOR_DEFAULT_DURATION_MS;
        if (hour<0 || hour>23 || minute<0 || minute>59 || second<0 || second>59 || !logic::durationValid(duration)) {
            storageReady=false; setDefaultSchedule(); return;
        }
        schedule[i] = {slot["active"] | false, uint8_t(hour), uint8_t(minute), uint8_t(second), uint16_t(duration), slot["lastRunDate"] | 0U};
        strlcpy(schedule[i].foodName,slot["foodName"] | "",sizeof(schedule[i].foodName));
        schedule[i].portionGrams=slot["portionGrams"] | 0.0f;
        if(schedule[i].foodName[0] && (schedule[i].portionGrams<=0 || schedule[i].portionGrams>500)) {storageReady=false;return;}
        // Legacy schedules require explicit review before enabling.
        if (!slot.containsKey("lastRunDate")) schedule[i].active = false;
    }
}

bool saveSchedule() {
    DynamicJsonDocument doc(12288);
    JsonArray arr = doc.createNestedArray("slots");
    for (const auto& item : schedule) {
        JsonObject slot = arr.createNestedObject();
        slot["active"]=item.active; slot["hour"]=item.hour; slot["minute"]=item.minute; slot["second"]=item.second;
        slot["duration"]=item.durationMs; slot["lastRunDate"]=item.lastRunDate;
        slot["foodName"]=item.foodName; slot["portionGrams"]=item.portionGrams;
    }
    return atomicJson(SCHEDULE_FILE, doc);
}

void addLogEntry(uint8_t type, const char* message) {
    auto& entry=eventLog[logIndex];
    entry.timestamp=utcNow(); entry.uptime=millis(); entry.eventType=type;
    strlcpy(entry.message,message,sizeof(entry.message));
    logIndex=(logIndex+1)%MAX_LOG_ENTRIES;
    if (logCount<MAX_LOG_ENTRIES) ++logCount;
    logsDirty=true;
    Serial.printf("Event %u: %s\n",type,message);
    // Never perform flash I/O while the motor is energized.
    if (!motorRunning) saveLogToFile();
}

bool saveLogToFile() {
    if (!logsDirty) return true;
    DynamicJsonDocument doc(16384);
    JsonArray arr=doc.createNestedArray("logs");
    for (unsigned i=0;i<logCount;i++) {
        const auto& e=eventLog[(logic::oldest(logIndex,logCount,MAX_LOG_ENTRIES)+i)%MAX_LOG_ENTRIES];
        JsonObject o=arr.createNestedObject();
        o["ts"]=e.timestamp; o["uptime"]=e.uptime; o["type"]=e.eventType; o["msg"]=e.message;
    }
    bool ok=atomicJson(LOG_FILE,doc);
    if(ok) logsDirty=false;
    return ok;
}

void loadLogFromFile() {
    File f=LittleFS.open(LOG_FILE,"r");
    if(!f) return;
    DynamicJsonDocument doc(16384);
    if(deserializeJson(doc,f)) { Serial.println("Log unreadable"); return; }
    logCount=0;
    for(JsonObject o:doc["logs"].as<JsonArray>()) {
        if(logCount==MAX_LOG_ENTRIES) break;
        auto& e=eventLog[logCount++];
        // Legacy firmware stored millis() in ts, not an epoch timestamp.
        e.timestamp=o.containsKey("uptime") ? (o["ts"] | 0U) : 0U;
        e.uptime=o.containsKey("uptime") ? (o["uptime"] | 0U) : (o["ts"] | 0U);
        e.eventType=o["type"] | 0;
        strlcpy(e.message,o["msg"] | "",sizeof(e.message));
    }
    logIndex=logCount%MAX_LOG_ENTRIES;
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
    config.deepSleepMinutes = 0;
    strlcpy(config.timezone, "UTC", sizeof(config.timezone));
    strlcpy(config.deviceName, "FeedMate", sizeof(config.deviceName));
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
    for (int i = 0; i < MAX_SCHEDULE_ENTRIES; i++) {
        schedule[i].active = false;
        schedule[i].hour = 0;
        schedule[i].minute = 0;
        schedule[i].second = 0;
        schedule[i].durationMs = MOTOR_DEFAULT_DURATION_MS;
        schedule[i].lastRunDate = 0;
        schedule[i].foodName[0] = 0;
        schedule[i].portionGrams = 0;
    }
}
