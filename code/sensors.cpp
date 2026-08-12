#include "config.h"

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

    // Reset rotation counter
    rotationCount = 0;

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
                digitalWrite(PIN_LED_BUILTIN, HIGH);
                Serial.println("JAM DETECTED - Motor stopped!");
                addLogEntry(EVT_FEEDING_JAM, "Jam protection triggered");

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

void checkSensors() {
    // Light barrier (food level)
    if (config.lightBarrierEnabled) {
        // Assuming sensor outputs HIGH when food present, LOW when empty
        bool foodPresent = digitalRead(PIN_LIGHT_BARRIER);

        if (!foodPresent) {
            if (foodLevelPercent > 0) foodLevelPercent--;
        } else {
            if (foodLevelPercent < 100) foodLevelPercent += 2;
        }

        // Alert if below threshold
        if (foodLevelPercent <= config.lightBarrierThreshold && foodLevelPercent > 0) {
            static bool alertSent = false;
            if (!alertSent) {
                alertSent = true;
                Serial.println("WARNING: Food level low!");
                addLogEntry(EVT_SENSOR_LOW_FOOD, "Food level below threshold");
                
                if (config.mqttEnabled && mqttClient.connected()) {
                    mqttClient.publish(
                        (String(config.mqttPrefix) + "/sensor/food_level").c_str(),
                        String(foodLevelPercent).c_str());
                    mqttClient.publish(
                        (String(config.mqttPrefix) + "/events/low_food").c_str(),
                        "Food level critical");
                }
            }
        } else if (foodLevelPercent == 0) {
            // Reset would be handled by manual reset button
        } else {
            // Reset alert flag when food level is good again
            if (foodLevelPercent > config.lightBarrierThreshold + 10) {
                // Find and reset the alert flag (implement properly)
            }
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
                addLogEntry(EVT_FEEDING_JAM, "Max rotations reached");
            }
        }
        lastHallState = hallState;
    }
}

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
                digitalWrite(PIN_LED_BUILTIN, LOW);
                Serial.println("Jam reset via manual button");
                addLogEntry(EVT_MANUAL_FEED, "Jam reset via button");

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