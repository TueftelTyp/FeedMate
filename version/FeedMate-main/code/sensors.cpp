#include "config.h"
static volatile uint32_t hallEdges=0,lastHallMicros=0;
static uint32_t lastPulse=0;
static bool jamTiming=false;
static bool hallAttached=false;
static uint16_t pendingDuration=0;
static uint8_t pendingRotations=255,activeRotations=0;
bool requestMotor(uint16_t durationMs) { return requestMotor(durationMs,255); }
bool requestMotor(uint16_t durationMs,uint8_t targetRotations) {
    if(pendingDuration || motorRunning || jamActive || !storageReady || digitalRead(PIN_JAM_SENSOR) || !logic::durationValid(durationMs)) return false;
    if(targetRotations!=255 && ((!config.endSwitchEnabled && targetRotations) || targetRotations>config.maxRotations)) return false;
    pendingDuration=durationMs;
    pendingRotations=targetRotations;
    return true;
}
void serviceMotorRequest() {
    if(!pendingDuration) return;
    uint16_t duration=pendingDuration; pendingDuration=0;
    uint8_t target=pendingRotations; pendingRotations=255;
    if(startMotor(duration)) { if(target!=255) activeRotations=target; }
    else addLogEntry(EVT_FEEDING_ABORTED,"Queued feed rejected by motor controller");
}
void IRAM_ATTR hallInterrupt() {
    uint32_t now=micros();
    if(uint32_t(now-lastHallMicros)>=2000) { ++hallEdges; lastHallMicros=now; }
}
bool startMotor(uint16_t durationMs) {
    if(!storageReady || motorRunning || jamActive || digitalRead(PIN_JAM_SENSOR) || !logic::durationValid(durationMs)) return false;
    if(config.endSwitchEnabled && (!config.maxRotations || !config.rotationsPerPortion || config.rotationsPerPortion>config.maxRotations)) return false;
    addLogEntry(EVT_FEEDING_STARTED,"Feeding reserved; motor starting");
    if(logsDirty) return false;
    if(config.endSwitchEnabled && !hallAttached) { attachInterrupt(digitalPinToInterrupt(PIN_HALL_SENSOR),hallInterrupt,FALLING); hallAttached=true; }
    if(!config.endSwitchEnabled && hallAttached) { detachInterrupt(digitalPinToInterrupt(PIN_HALL_SENSOR)); hallAttached=false; }
    noInterrupts(); hallEdges=0; lastHallMicros=micros(); interrupts();
    rotationCount=0; jamTiming=false; motorStartTime=millis(); lastPulse=motorStartTime;
    motorDuration=durationMs; motorRunning=true;
    activeRotations=config.endSwitchEnabled?config.rotationsPerPortion:0;
    digitalWrite(PIN_MOTOR_GATE,HIGH);
    return true;
}
void stopMotor(bool success,const char* reason) {
    digitalWrite(PIN_MOTOR_GATE,LOW);
    if(!motorRunning) return;
    motorRunning=false;
    addLogEntry(success?EVT_FEEDING_OK:EVT_FEEDING_ABORTED,reason);
}
static void fault(const char* reason) {
    jamActive=true; digitalWrite(PIN_LED_BUILTIN,STATUS_LED_ON);
    stopMotor(false,reason);
}
bool resetFault() {
    if(motorRunning || digitalRead(PIN_JAM_SENSOR)) return false;
    jamActive=false; jamTiming=false; digitalWrite(PIN_LED_BUILTIN,STATUS_LED_OFF); return true;
}
void serviceMotor() {
    if(!motorRunning) return;
    uint32_t now=millis();
    if(digitalRead(PIN_JAM_SENSOR)) {
        if(!jamTiming) { jamTiming=true; jamDetectedAt=now; }
        if(logic::expired(now,jamDetectedAt,constrain(config.jamTimeout,50,1000))) { fault("Jam input active"); return; }
    } else jamTiming=false;
    if(config.endSwitchEnabled) {
        noInterrupts(); uint32_t count=hallEdges; interrupts();
        if(count!=rotationCount) { rotationCount=count; lastPulse=now; }
        if(count>config.maxRotations) { fault("Hall rotation limit exceeded"); return; }
        if(activeRotations && count>=activeRotations) { stopMotor(true,"Target rotations reached; quantity not measured"); return; }
        if(logic::expired(now,lastPulse,1500)) { fault("No Hall pulses for 1500 ms"); return; }
    }
    if(logic::expired(now,motorStartTime,motorDuration)) {
        if(config.endSwitchEnabled && activeRotations) fault("Time limit before target rotations");
        else stopMotor(true,"Timed motor cycle completed; quantity not measured");
    }
}
void checkSensors() {
    if(!config.lightBarrierEnabled) return;
    static bool candidate=false,stable=false,initialized=false;
    static uint32_t changed=0;
    bool value=digitalRead(PIN_LIGHT_BARRIER);
    if(value!=candidate) { candidate=value; changed=millis(); }
    if(millis()-changed>=500 && (!initialized || stable!=candidate)) {
        stable=candidate; initialized=true; foodLevelPercent=stable?100:0;
        if(!stable) addLogEntry(EVT_SENSOR_LOW_FOOD,"Light barrier reports no food");
    }
}
void handleManualButton() {
    static bool candidate=HIGH,stable=HIGH;
    static uint32_t changed=0;
    bool raw=digitalRead(PIN_MANUAL_BUTTON);
    if(raw!=candidate) { candidate=raw; changed=millis(); }
    if(raw!=stable && millis()-changed>=50) {
        stable=raw;
        if(!stable) { if(jamActive) resetFault(); else startMotor(MOTOR_DEFAULT_DURATION_MS); }
    }
}
