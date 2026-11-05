#include "config.h"
RTC_DS3231 rtc;
FeedMateWebServer webServer(80);
WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);
WiFiUDP ntpUDP;
bool motorRunning=false, jamActive=false, powerOnline=true;
unsigned long motorStartTime=0,motorDuration=0,jamDetectedAt=0,lastMqttPublish=0,lastScheduleCheck=0;
uint32_t rotationCount=0;
uint8_t foodLevelPercent=0;
bool isAuthenticated=false;
unsigned long lastAuthTime=0;
bool clockValid=false,rtcPresent=false;
String bootNonce;

uint32_t utcNow() {
    time_t now=time(nullptr);
    return clockValid && now>=1704067200 && now<4102444800LL ? uint32_t(now):0;
}
void setupRTC() {
    Wire.begin(PIN_RTC_SDA,PIN_RTC_SCL);
    rtcPresent=rtc.begin();
    if(rtcPresent && !rtc.lostPower()) {
        DateTime now=rtc.now();
        if(now.isValid() && now.year()>=2024 && now.year()<2100) {
            timeval tv={time_t(now.unixtime()),0}; settimeofday(&tv,nullptr); clockValid=true;
        }
    }
    // RTC stores UTC. POSIX timezone rules affect schedules and display only.
#if defined(ESP32)
    configTzTime(config.timezone,"pool.ntp.org","time.nist.gov");
#else
    configTime(config.timezone,"pool.ntp.org","time.nist.gov");
#endif
}
void serviceClock() {
    time_t now=time(nullptr);
    if(now<1704067200 || now>=4102444800LL) { clockValid=false; return; }
    clockValid=true;
    static uint32_t last=0;
    if(rtcPresent && millis()-last>=60000) {
        last=millis();
        Wire.beginTransmission(0x68);
        if(Wire.endTransmission()!=0) { rtcPresent=false; return; }
        DateTime rtcTime=rtc.now();
        if(rtc.lostPower() || !rtcTime.isValid() || labs(long(now)-long(rtcTime.unixtime()))>2)
            rtc.adjust(DateTime(uint32_t(now)));
    }
}
void setupWiFi() {
    WiFi.mode(WIFI_STA);
#if defined(ESP32)
    WiFi.setHostname(WIFI_HOSTNAME);
#else
    WiFi.hostname(WIFI_HOSTNAME);
#endif
    WiFi.setAutoReconnect(true);
    if(config.wifiSsid[0]) WiFi.begin(config.wifiSsid,config.wifiPassword);
    else startFallbackAP();
}
void startFallbackAP() {
    WiFi.mode(WIFI_AP_STA);
    String password=randomHex(8);
    String name=String("FeedMate-")+deviceId();
    WiFi.softAP(name.c_str(),password.c_str());
    Serial.printf("Setup AP: %s Password: %s\n",name.c_str(),password.c_str());
}
void checkPowerStatus() {
    int raw=analogRead(PIN_POWER_SENSE);
    bool online=powerOnline ? raw>450 : raw>550;
    if(online!=powerOnline) { powerOnline=online; addLogEntry(online?EVT_POWER_RESTORED:EVT_POWER_OUTAGE,online?"Power restored":"Power outage"); }
}
void setup() {
    digitalWrite(PIN_MOTOR_GATE,LOW); pinMode(PIN_MOTOR_GATE,OUTPUT);
    Serial.begin(115200);
#if defined(ESP32)
    analogReadResolution(10); // Shared raw ADC thresholds; calibrate actual divider.
    analogSetPinAttenuation(PIN_POWER_SENSE,ADC_11db);
#endif
    pinMode(PIN_JAM_SENSOR,INPUT_PULLUP); pinMode(PIN_LIGHT_BARRIER,INPUT);
    pinMode(PIN_HALL_SENSOR,INPUT); pinMode(PIN_MANUAL_BUTTON,INPUT_PULLUP);
    pinMode(PIN_LED_BUILTIN,OUTPUT); digitalWrite(PIN_LED_BUILTIN,STATUS_LED_OFF);
    setupLittleFS(); loadConfig(); loadSchedule(); loadLogFromFile();
    // RF must be active before using the hardware RNG on either platform.
    WiFi.mode(WIFI_STA);
    bootNonce=randomHex(16); setupAuth(); setupWiFi(); setupRTC(); setupWebServer(); setupMQTT();
    addLogEntry(EVT_BOOT,"Boot; interrupted feeds are never resumed");
}
void loop() {
    serviceMotor(); checkSensors(); handleManualButton();
    // Synchronous network and flash operations are excluded while running.
    if(motorRunning) { platformIdle(); return; }
    serviceClock();
    webServer.handleClient();
    serviceMotorRequest();
    if(motorRunning) return;
    serviceMQTT();
    serviceMotorRequest();
    if(motorRunning) return;
    if(millis()-lastScheduleCheck>=1000) { lastScheduleCheck=millis(); checkSchedule(); }
    if(motorRunning) return;
    static uint32_t lastSlow=0;
    if(millis()-lastSlow>=5000) { lastSlow=millis(); checkPowerStatus(); if(logsDirty) saveLogToFile(); }
    static bool fallback=false;
    if(!fallback && config.wifiSsid[0] && millis()>30000 && WiFi.status()!=WL_CONNECTED) { fallback=true; startFallbackAP(); }
    platformIdle();
}
