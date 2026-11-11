#include "config.h"

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
        
        // Home Assistant Auto-Discovery
        if (config.haDiscovery) {
            publishHaDiscovery();
        }
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
                addLogEntry(EVT_MANUAL_FEED, "Manual feed via MQTT");
            }
        }
    }
    else if (topicStr.endsWith("/control/feed_duration")) {
        uint16_t duration = payloadStr.toInt();
        if (duration > 0 && duration <= MOTOR_MAX_DURATION_MS) {
            if (!motorRunning && !jamActive) {
                startMotor(duration);
                addLogEntry(EVT_MANUAL_FEED, "Manual feed via MQTT (custom duration)");
            }
        }
    }
    else if (topicStr.endsWith("/control/reset_sensors")) {
        jamActive = false;
        rotationCount = 0;
        foodLevelPercent = 100;
        digitalWrite(PIN_LED_BUILTIN, LOW);
        Serial.println("Sensors reset via MQTT");
        addLogEntry(EVT_MANUAL_FEED, "Sensors reset via MQTT");
    }
}

void publishHaDiscovery() {
    // Home Assistant Auto-Discovery for sensors
    String prefix = config.mqttPrefix;
    
    // Feeding switch
    StaticJsonDocument<512> doc;
    doc["name"] = "FeedMate Feed Now";
    doc["command_topic"] = prefix + "/control/feed_now";
    doc["payload_on"] = "1";
    doc["payload_off"] = "0";
    doc["state_topic"] = prefix + "/status/motor_running";
    doc["unique_id"] = "feedmate_feed_now";
    
    char buffer[1024];
    serializeJson(doc, buffer);
    mqttClient.publish(("homeassistant/switch/" + prefix + "/feed/config").c_str(), buffer, true);
    
    // Food level sensor
    doc.clear();
    doc["name"] = "FeedMate Food Level";
    doc["state_topic"] = prefix + "/sensor/food_level";
    doc["unit_of_measurement"] = "%";
    doc["unique_id"] = "feedmate_food_level";
    
    serializeJson(doc, buffer);
    mqttClient.publish(("homeassistant/sensor/" + prefix + "/food_level/config").c_str(), buffer, true);
}

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
    mqttClient.publish((prefix + "/status/power").c_str(),
        powerOnline ? "1" : "0", true);

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
    
    // Battery (simulate or read from ADC)
    mqttClient.publish((prefix + "/status/battery").c_str(), "87");
}

void setupMQTT() {
    if (strlen(config.mqttServer) == 0) return;

    mqttClient.setServer(config.mqttServer, config.mqttPort);
    mqttClient.setCallback(mqttCallback);

    connectMQTT();
}