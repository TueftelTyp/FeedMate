#include "config.h"

// SHA256 implementation (simplified)
String sha256(const String& data) {
    // In production, use proper crypto library
    // This is just a placeholder
    return String(data.length()) + "_" + data.substring(0, 8);
}

bool checkAuth() {
    if (webServer.hasHeader("Authorization")) {
        String auth = webServer.header("Authorization");
        if (auth.startsWith("Bearer ")) {
            String token = auth.substring(7);
            // Validate token (implement proper JWT or session)
            return token.length() > 0;
        }
    }
    return false;
}

void handleLogin() {
    if (webServer.method() == HTTP_POST) {
        String body = webServer.arg("plain");
        StaticJsonDocument<512> doc;
        DeserializationError error = deserializeJson(doc, body);

        if (!error) {
            const char* password = doc["password"];
            String hash = sha256(password);
            
            if (strlen(config.adminPassword) == 0 || hash == config.adminPassword) {
                isAuthenticated = true;
                lastAuthTime = millis();
                
                StaticJsonDocument<256> response;
                response["success"] = true;
                response["token"] = "session_token_" + String(millis());
                
                String responseStr;
                serializeJson(response, responseStr);
                webServer.send(200, "application/json", responseStr);
                return;
            }
        }
        
        webServer.send(401, "application/json", "{\"error\":\"Invalid credentials\"}");
    } else {
        webServer.send(405, "application/json", "{\"error\":\"Method not allowed\"}");
    }
}

void handleStatus() {
    StaticJsonDocument<1024> doc;
    
    doc["motorRunning"] = motorRunning;
    doc["jamActive"] = jamActive;
    doc["powerOnline"] = powerOnline;
    doc["foodLevel"] = foodLevelPercent;
    doc["rotations"] = rotationCount;
    doc["isAuthenticated"] = isAuthenticated;

    DateTime now = rtc.now();
    char timeBuf[20];
    sprintf(timeBuf, "%04d-%02d-%02d %02d:%02d:%02d",
        now.year(), now.month(), now.day(),
        now.hour(), now.minute(), now.second());
    doc["rtcTime"] = timeBuf;

    // Next feeding
    DateTime nextFeed = now;
    bool found = false;
    for (int i = 0; i < 8; i++) {
        if (schedule[i].active && !schedule[i].lastRunToday) {
            if (schedule[i].hour > nextFeed.hour() || 
                (schedule[i].hour == nextFeed.hour() && schedule[i].minute > nextFeed.minute())) {
                nextFeed = DateTime(now.year(), now.month(), now.day(), 
                                   schedule[i].hour, schedule[i].minute, 0);
                found = true;
            }
        }
    }
    
    if (found) {
        char nextBuf[20];
        sprintf(nextBuf, "%02d:%02d", nextFeed.hour(), nextFeed.minute());
        doc["nextFeed"] = nextBuf;
    }

    String response;
    serializeJson(doc, response);
    webServer.send(200, "application/json", response);
}

void handleFeed() {
    if (!checkAuth()) {
        webServer.send(401, "application/json", "{\"error\":\"Unauthorized\"}");
        return;
    }

    uint16_t duration = MOTOR_DEFAULT_DURATION_MS;
    if (webServer.hasArg("duration")) {
        duration = webServer.arg("duration").toInt();
        if (duration > MOTOR_MAX_DURATION_MS) duration = MOTOR_MAX_DURATION_MS;
    }

    if (!motorRunning && !jamActive) {
        startMotor(duration);
        addLogEntry(EVT_MANUAL_FEED, "Manual feeding triggered via web");
        webServer.send(200, "application/json", "{\"status\":\"started\"}");
    } else {
        webServer.send(409, "application/json", "{\"error\":\"Motor busy or jam active\"}");
    }
}

void handleScheduleGet() {
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
}

void handleSchedulePost() {
    if (!checkAuth()) {
        webServer.send(401, "application/json", "{\"error\":\"Unauthorized\"}");
        return;
    }

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
}

void handleConfigGet() {
    StaticJsonDocument<2048> doc;
    doc["wifiSsid"] = String(config.wifiSsid);
    doc["mqttServer"] = String(config.mqttServer);
    doc["mqttPort"] = config.mqttPort;
    doc["mqttEnabled"] = config.mqttEnabled;
    doc["jamThreshold"] = config.jamThreshold;
    doc["lightBarrierEnabled"] = config.lightBarrierEnabled;
    doc["endSwitchEnabled"] = config.endSwitchEnabled;
    doc["lightBarrierThreshold"] = config.lightBarrierThreshold;
    doc["rotationsPerPortion"] = config.rotationsPerPortion;
    doc["maxRotations"] = config.maxRotations;

    String response;
    serializeJson(doc, response);
    webServer.send(200, "application/json", response);
}

void handleConfigPost() {
    if (!checkAuth()) {
        webServer.send(401, "application/json", "{\"error\":\"Unauthorized\"}");
        return;
    }

    String body = webServer.arg("plain");
    StaticJsonDocument<2048> doc;
    DeserializationError error = deserializeJson(doc, body);

    if (!error) {
        if (doc.containsKey("wifiSsid")) strlcpy(config.wifiSsid, doc["wifiSsid"], sizeof(config.wifiSsid));
        if (doc.containsKey("mqttServer")) strlcpy(config.mqttServer, doc["mqttServer"], sizeof(config.mqttServer));
        if (doc.containsKey("mqttPort")) config.mqttPort = doc["mqttPort"];
        if (doc.containsKey("mqttEnabled")) config.mqttEnabled = doc["mqttEnabled"];
        if (doc.containsKey("jamThreshold")) config.jamThreshold = doc["jamThreshold"];
        if (doc.containsKey("lightBarrierEnabled")) config.lightBarrierEnabled = doc["lightBarrierEnabled"];
        if (doc.containsKey("endSwitchEnabled")) config.endSwitchEnabled = doc["endSwitchEnabled"];
        if (doc.containsKey("lightBarrierThreshold")) config.lightBarrierThreshold = doc["lightBarrierThreshold"];
        if (doc.containsKey("rotationsPerPortion")) config.rotationsPerPortion = doc["rotationsPerPortion"];
        if (doc.containsKey("maxRotations")) config.maxRotations = doc["maxRotations"];
        
        saveConfig();
        webServer.send(200, "application/json", "{\"status\":\"saved\"}");
    } else {
        webServer.send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
    }
}

void handleLogsGet() {
    StaticJsonDocument<4096> doc;
    JsonArray arr = doc.createNestedArray("logs");
    
    for (int i = 0; i < logCount; i++) {
        uint8_t idx = (logIndex + i) % MAX_LOG_ENTRIES;
        JsonObject entry = arr.createNestedObject();
        entry["ts"] = eventLog[idx].timestamp;
        entry["type"] = eventLog[idx].eventType;
        entry["msg"] = eventLog[idx].message;
    }
    
    String response;
    serializeJson(doc, response);
    webServer.send(200, "application/json", response);
}

void handleHTML() {
    File file = LittleFS.open(HTML_FILE, "r");
    if (!file) {
        webServer.send(404, "text/plain", "HTML file not found. Please upload via LittleFS uploader.");
        return;
    }
    
    String contentType = "text/html";
    if (file.name().endsWith(".gz")) {
        contentType = "text/html";
        webServer.sendHeader("Content-Encoding", "gzip");
    }
    
    webServer.streamFile(file, contentType);
    file.close();
}

void setupWebServer() {
    // Static files
    webServer.on("/", HTTP_GET, handleHTML);
    webServer.on("/index.html", HTTP_GET, handleHTML);
    webServer.on("/index.html.gz", HTTP_GET, handleHTML);
    
    // API endpoints
    webServer.on("/api/login", HTTP_POST, handleLogin);
    webServer.on("/api/status", HTTP_GET, handleStatus);
    webServer.on("/api/feed", HTTP_POST, handleFeed);
    webServer.on("/api/schedule", HTTP_GET, handleScheduleGet);
    webServer.on("/api/schedule", HTTP_POST, handleSchedulePost);
    webServer.on("/api/config", HTTP_GET, handleConfigGet);
    webServer.on("/api/config", HTTP_POST, handleConfigPost);
    webServer.on("/api/logs", HTTP_GET, handleLogsGet);
    
    // OTA update
    httpUpdater.setup(&webServer, "/update", "admin", "feedmate");
    
    webServer.begin();
    Serial.println("Web server started");
}