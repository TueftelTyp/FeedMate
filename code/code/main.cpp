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
String managementApSsid;
bool managementApStarted=false;
uint32_t managementApStarts=0,managementApFailures=0;
static uint32_t lastFallbackAttempt=0;
static bool stationConnectPending=false;
static uint32_t stationConnectAfter=0;

uint32_t utcNow() {
    time_t now=time(nullptr);
    return clockValid && now>=1704067200 && now<4102444800LL ? uint32_t(now):0;
}
struct ZoneRule { const char* name; const char* rule; };
static const ZoneRule timezoneRules[] = {
        {"UTC","UTC0"}, {"Europe/London","GMT0BST,M3.5.0/1,M10.5.0"},
        {"Europe/Lisbon","WET0WEST,M3.5.0/1,M10.5.0"},
        {"Europe/Berlin","CET-1CEST,M3.5.0,M10.5.0/3"},
        {"Europe/Helsinki","EET-2EEST,M3.5.0/3,M10.5.0/4"},
        {"Europe/Moscow","MSK-3"},
        {"America/New_York","EST5EDT,M3.2.0/2,M11.1.0/2"},
        {"America/Chicago","CST6CDT,M3.2.0/2,M11.1.0/2"},
        {"America/Denver","MST7MDT,M3.2.0/2,M11.1.0/2"},
        {"America/Los_Angeles","PST8PDT,M3.2.0/2,M11.1.0/2"},
        {"America/Phoenix","MST7"},
        {"America/Anchorage","AKST9AKDT,M3.2.0/2,M11.1.0/2"},
        {"Pacific/Honolulu","HST10"},
        {"America/Argentina/Buenos_Aires","ART3"},
        {"America/Sao_Paulo","BRT3"}, {"Africa/Johannesburg","SAST-2"},
        {"Asia/Dubai","GST-4"}, {"Asia/Kolkata","IST-5:30"},
        {"Asia/Bangkok","ICT-7"}, {"Asia/Shanghai","CST-8"},
        {"Asia/Hong_Kong","HKT-8"}, {"Asia/Singapore","SGT-8"},
        {"Asia/Tokyo","JST-9"}, {"Asia/Seoul","KST-9"},
        {"Australia/Perth","AWST-8"}, {"Australia/Darwin","ACST-9:30"},
        {"Australia/Brisbane","AEST-10"},
        {"Australia/Adelaide","ACST-9:30ACDT,M10.1.0,M4.1.0/3"},
        {"Australia/Sydney","AEST-10AEDT,M10.1.0,M4.1.0/3"},
        {"Pacific/Auckland","NZST-12NZDT,M9.5.0,M4.1.0/3"}
};
const char* timezoneRule(const char* zone) {
    if(!zone) return nullptr;
    for(const auto& item:timezoneRules)
        if(strcmp(zone,item.name)==0 || strcmp(zone,item.rule)==0) return item.rule;
    return nullptr;
}
bool timezoneSupported(const char* zone) { return timezoneRule(zone)!=nullptr; }
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
    const char* rule=timezoneRule(config.timezone);
    if(!rule) rule="UTC0";
#if defined(ESP32)
    configTzTime(rule,"pool.ntp.org","time.nist.gov");
#else
    configTime(rule,"pool.ntp.org","time.nist.gov");
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
    String hostname;
    for(const char* p=config.deviceName; *p && hostname.length()<32; ++p) {
        const char c=*p;
        if((c>='a'&&c<='z')||(c>='A'&&c<='Z')||(c>='0'&&c<='9')) hostname+=char(tolower(c));
        else if((c==' '||c=='-'||c=='_') && hostname.length() && hostname[hostname.length()-1]!='-') hostname+='-';
    }
    while(hostname.endsWith("-")) hostname.remove(hostname.length()-1);
    if(!hostname.length()) hostname=WIFI_HOSTNAME;
#if defined(ESP32)
    // Arduino-ESP32 requires the hostname before the first Wi-Fi mode change.
    if(!WiFi.setHostname(hostname.c_str())) Serial.println("STA hostname rejected");
#else
    WiFi.hostname(hostname.c_str());
#endif
    WiFi.setAutoReconnect(true);
    // Keep the AP interface and its channel stable while a phone is using it.
    // The saved home network is attempted only after setup clients leave.
    stationConnectPending=config.wifiSsid[0]!=0;
    stationConnectAfter=millis()+5000;
    startFallbackAP();
}
void startFallbackAP() {
    lastFallbackAttempt=millis();
    managementApSsid=String("FeedMate-")+deviceId();
    const auto currentMode=WiFi.getMode();
    const bool alreadyActive=(currentMode==WIFI_AP || currentMode==WIFI_AP_STA) &&
        WiFi.softAPIP()!=IPAddress(0,0,0,0) && WiFi.softAPSSID()==managementApSsid;
    if(alreadyActive) { managementApStarted=true; return; }
    // Use AP-only mode when no router is configured. Keep AP+STA when saved
    // credentials exist; never stop Wi-Fi during AP recovery.
    const auto desiredMode=config.wifiSsid[0]?WIFI_AP_STA:WIFI_AP;
    if(currentMode!=desiredMode) WiFi.mode(desiredMode);
#if defined(ESP32)
    WiFi.setSleep(false);
    if(!WiFi.setTxPower(WIFI_POWER_17dBm))
        Serial.println("Management AP TX power configuration failed");
#endif
    delay(100);
    const IPAddress local(192,168,4,1),gateway(192,168,4,1),mask(255,255,255,0);
    bool configured=WiFi.softAPConfig(local,gateway,mask);
    bool started=WiFi.softAP(managementApSsid.c_str(),INITIAL_AP_PASSWORD,1,false,4);
    delay(250);
    const auto mode=WiFi.getMode();
    managementApStarted=started &&
        (mode==WIFI_AP || mode==WIFI_AP_STA) &&
        WiFi.softAPIP()!=IPAddress(0,0,0,0) &&
        WiFi.softAPSSID()==managementApSsid;
    if(managementApStarted) ++managementApStarts; else ++managementApFailures;
    Serial.printf("Management AP %s: %s Password: %s Channel: %u IP: %s mode=%u\n",
                  managementApStarted?"started":"FAILED",managementApSsid.c_str(),
                  INITIAL_AP_PASSWORD,unsigned(WiFi.channel()),WiFi.softAPIP().toString().c_str(),unsigned(mode));
    if(!configured) Serial.println("Management AP address was already configured or could not be changed");
}
static void serviceStationConnection() {
    if(!stationConnectPending || int32_t(millis()-stationConnectAfter)<0) return;
    if(WiFi.softAPgetStationNum()!=0) return;
    stationConnectPending=false;
    Serial.printf("Starting saved Wi-Fi connection: %s\n",config.wifiSsid);
    WiFi.begin(config.wifiSsid,config.wifiPassword);
}
void serviceManagementAP() {
    static uint32_t lastCheck=0,lastReport=0;
    if(uint32_t(millis()-lastCheck)<2000) return;
    lastCheck=millis();
    const auto mode=WiFi.getMode();
    const bool active=(mode==WIFI_AP || mode==WIFI_AP_STA) &&
        WiFi.softAPIP()!=IPAddress(0,0,0,0) &&
        WiFi.softAPSSID()==managementApSsid;
    managementApStarted=active;
    if(!active && uint32_t(millis()-lastFallbackAttempt)>=2000) {
        Serial.println("Management AP health check failed; restarting AP");
        startFallbackAP();
    }
    if(managementApStarted && uint32_t(millis()-lastReport)>=30000) {
        lastReport=millis();
        Serial.printf("Management AP active: %s IP: %s Clients: %u Starts: %lu Failures: %lu\n",
                      managementApSsid.c_str(),WiFi.softAPIP().toString().c_str(),
                      unsigned(WiFi.softAPgetStationNum()),
                      static_cast<unsigned long>(managementApStarts),
                      static_cast<unsigned long>(managementApFailures));
    }
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
    setupStatusLed();
    setupLittleFS(); loadConfig(); loadSchedule(); loadLogFromFile();
    bootNonce=randomHex(16); setupAuth();
    // Set the configured hostname before starting Wi-Fi; start the setup AP
    // before RTC, web-server and MQTT setup, without a later WIFI_OFF cycle.
    setupWiFi();
    delay(1750); // Give the AP time to emit beacons before the first loop pass.
    setupRTC(); setupWebServer(); setupMQTT();
    addLogEntry(EVT_BOOT,"Boot; interrupted feeds are never resumed");
}
void loop() {
    serviceMotor(); checkSensors(); handleManualButton();
    // Synchronous network and flash operations are excluded while running.
    if(motorRunning) { platformIdle(); return; }
    serviceClock();
    serviceManagementAP();
    serviceStationConnection();
    if(millis()-lastScheduleCheck>=200) { lastScheduleCheck=millis(); checkSchedule(); }
    if(motorRunning) return;
    webServer.handleClient();
    serviceDeferredRestart();
    serviceMotorRequest();
    if(motorRunning) return;
    serviceMQTT();
    serviceMotorRequest();
    if(motorRunning) return;
    static uint32_t lastSlow=0;
    if(millis()-lastSlow>=5000) { lastSlow=millis(); checkPowerStatus(); if(logsDirty) saveLogToFile(); }
    platformIdle();
}
