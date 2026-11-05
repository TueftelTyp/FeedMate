#include "config.h"
#include <memory>
#if defined(ESP32)
#include <mbedtls/sha256.h>
#include <esp_system.h>
#else
#include <bearssl/bearssl.h>
extern "C" {
#include <user_interface.h>
}
#endif
static String sessionToken;
static uint32_t sessionStart=0,lastLoginAttempt=0;
static bool attemptedLogin=false;
String randomHex(size_t bytes) {
    String out; out.reserve(bytes*2);
    for(size_t i=0;i<bytes;i++) {
        uint8_t b;
#if defined(ESP32)
        esp_fill_random(&b,1);
#else
        os_get_random(&b,1);
#endif
        char hex[3]; snprintf(hex,sizeof(hex),"%02x",b); out+=hex;
    }
    return out;
}
static String passwordHash(const String& password) {
    // Device-specific salt; high-entropy generated password is the initial credential.
    String input=String("FeedMate:")+deviceId()+":"+password;
    uint8_t digest[32];
#if defined(ESP32)
    if(mbedtls_sha256_ret(reinterpret_cast<const unsigned char*>(input.c_str()),input.length(),digest,0)!=0) return "";
#else
    br_sha256_context ctx;
    br_sha256_init(&ctx); br_sha256_update(&ctx,input.c_str(),input.length()); br_sha256_out(&ctx,digest);
#endif
    String hash; hash.reserve(64);
    for(auto b:digest) { char hex[3]; snprintf(hex,sizeof(hex),"%02x",b); hash+=hex; }
    return hash;
}
static bool same(const String& a,const String& b) {
    return logic::secureEqual(a.c_str(),b.c_str());
}
void setupAuth() {
    // Old placeholder hashes cannot be migrated securely.
    if(strlen(config.adminPassword)!=64) {
        String password=randomHex(16);
        strlcpy(config.adminPassword,passwordHash(password).c_str(),sizeof(config.adminPassword));
        if(!saveConfig()) { config.adminPassword[0]=0; storageReady=false; return; }
        Serial.printf("NEW ADMIN PASSWORD (store securely): %s\n",password.c_str());
    }
}
static void error(int status,const char* message) { webServer.send(status,"application/json",String("{\"error\":\"")+message+"\"}"); }
static bool auth() {
    webServer.sendHeader("Cache-Control","no-store");
    if(!logic::sessionValid(sessionToken.c_str(),webServer.header("Authorization").c_str(),millis(),sessionStart)) { error(401,"Unauthorized"); return false; }
    return true;
}
static void sendJson(const JsonDocument& doc) {
    if(doc.overflowed()) { error(500,"Response too large"); return; }
    String body; serializeJson(doc,body); webServer.sendHeader("Cache-Control","no-store"); webServer.send(200,"application/json",body);
}
static bool body(JsonDocument& doc) {
    if(webServer.arg("plain").length()>4096 || deserializeJson(doc,webServer.arg("plain")) || !doc.is<JsonObject>()) { error(400,"Invalid JSON object"); return false; }
    return true;
}
static void login() {
    if(attemptedLogin && millis()-lastLoginAttempt<2000) { error(429,"Wait before retrying"); return; }
    attemptedLogin=true; lastLoginAttempt=millis();
    StaticJsonDocument<512> doc;
    if(!body(doc)) return;
    if(!doc["password"].is<const char*>() || !config.adminPassword[0] || !same(passwordHash(doc["password"].as<String>()),config.adminPassword)) { error(401,"Invalid credentials"); return; }
    sessionToken=randomHex(32); sessionStart=millis();
    doc.clear(); doc["token"]=sessionToken; doc["expiresIn"]=1800; sendJson(doc);
}
static void status() {
    if(!auth()) return;
    StaticJsonDocument<1024> doc;
    doc["motorRunning"]=motorRunning; doc["jamActive"]=jamActive; doc["powerOnline"]=powerOnline;
    doc["platform"]=platformName(); doc["deviceId"]=deviceId(); doc["freeHeap"]=ESP.getFreeHeap();
    doc["clockValid"]=clockValid; doc["rtcPresent"]=rtcPresent; doc["storageReady"]=storageReady;
    doc["logsPending"]=logsDirty; doc["utc"]=utcNow(); doc["rotations"]=rotationCount;
    doc["foodSensorEnabled"]=config.lightBarrierEnabled; doc["foodPresent"]=foodLevelPercent==100;
    doc["mqttConnected"]=mqttClient.connected(); doc["bootNonce"]=bootNonce;
    time_t epoch=utcNow(); struct tm local; localtime_r(&epoch,&local); char formatted[32];
    strftime(formatted,sizeof(formatted),"%Y-%m-%d %H:%M:%S",&local);
    if(clockValid) doc["localTime"]=formatted;
    sendJson(doc);
}
static void feed() {
    if(!auth()) return;
    StaticJsonDocument<256> doc; if(!body(doc)) return;
    if(!doc["duration"].is<long>() || !logic::durationValid(doc["duration"].as<long>())) { error(400,"Duration must be 1..30000 ms"); return; }
    if(!requestMotor(doc["duration"].as<uint16_t>())) { error(409,"Motor busy, fault or storage unavailable"); return; }
    webServer.send(202,"application/json","{\"status\":\"queued\"}");
}
static void getSchedule() {
    if(!auth()) return;
    DynamicJsonDocument doc(3072); JsonArray slots=doc.createNestedArray("slots");
    for(const auto& s:schedule) { JsonObject o=slots.createNestedObject(); o["active"]=s.active; o["hour"]=s.hour; o["minute"]=s.minute; o["duration"]=s.durationMs; o["lastRunDate"]=s.lastRunDate; }
    sendJson(doc);
}
static void postSchedule() {
    if(!auth()) return;
    DynamicJsonDocument doc(3072); if(!body(doc)) return;
    if(!doc["slots"].is<JsonArray>() || doc["slots"].size()!=8) { error(400,"Exactly eight slots required"); return; }
    FeedingSlot proposed[8]; memcpy(proposed,schedule,sizeof(schedule));
    for(int i=0;i<8;i++) {
        JsonObject o=doc["slots"][i];
        if(!o["active"].is<bool>() || !o["hour"].is<int>() || !o["minute"].is<int>() || !o["duration"].is<long>() ||
           o["hour"].as<int>()<0 || o["hour"].as<int>()>23 || o["minute"].as<int>()<0 || o["minute"].as<int>()>59 || !logic::durationValid(o["duration"].as<long>())) { error(400,"Invalid slot"); return; }
        proposed[i].active=o["active"]; proposed[i].hour=o["hour"]; proposed[i].minute=o["minute"]; proposed[i].durationMs=o["duration"];
        // Editing a slot never erases its reservation date.
    }
    FeedingSlot previous[8]; memcpy(previous,schedule,sizeof(schedule)); memcpy(schedule,proposed,sizeof(schedule));
    if(!saveSchedule()) { memcpy(schedule,previous,sizeof(schedule)); error(503,"Schedule commit failed"); return; }
    getSchedule();
}
static void getConfig() {
    if(!auth()) return;
    DynamicJsonDocument doc(2048);
    doc["wifiSsid"]=config.wifiSsid; doc["mqttServer"]=config.mqttServer; doc["mqttPort"]=config.mqttPort;
    doc["mqttUser"]=config.mqttUser; doc["mqttPrefix"]=config.mqttPrefix; doc["mqttEnabled"]=config.mqttEnabled;
    doc["haDiscovery"]=config.haDiscovery; doc["timezone"]=config.timezone; doc["jamTimeout"]=config.jamTimeout;
    doc["lightBarrierEnabled"]=config.lightBarrierEnabled; doc["endSwitchEnabled"]=config.endSwitchEnabled;
    doc["rotationsPerPortion"]=config.rotationsPerPortion; doc["maxRotations"]=config.maxRotations;
    sendJson(doc);
}
static void postConfig() {
    if(!auth()) return;
    DynamicJsonDocument doc(4096); if(!body(doc)) return;
    std::unique_ptr<FeedMateConfig> candidate(new FeedMateConfig(config));
    FeedMateConfig& proposed=*candidate;
    bool valid=true;
    auto stringField=[&](const char* key,char* dest,size_t cap) { if(!doc.containsKey(key)) return; if(!doc[key].is<const char*>() || strlen(doc[key])>=cap) valid=false; else strlcpy(dest,doc[key],cap); };
    stringField("wifiSsid",proposed.wifiSsid,sizeof(proposed.wifiSsid)); stringField("wifiPassword",proposed.wifiPassword,sizeof(proposed.wifiPassword));
    stringField("mqttServer",proposed.mqttServer,sizeof(proposed.mqttServer)); stringField("mqttUser",proposed.mqttUser,sizeof(proposed.mqttUser));
    stringField("mqttPassword",proposed.mqttPassword,sizeof(proposed.mqttPassword)); stringField("mqttPrefix",proposed.mqttPrefix,sizeof(proposed.mqttPrefix));
    stringField("timezone",proposed.timezone,sizeof(proposed.timezone));
    auto number=[&](const char* key,long low,long high,long old)->long { if(!doc.containsKey(key)) return old; if(!doc[key].is<long>() || doc[key].as<long>()<low || doc[key].as<long>()>high) {valid=false;return old;} return doc[key].as<long>(); };
    proposed.mqttPort=number("mqttPort",1,65535,proposed.mqttPort); proposed.jamTimeout=number("jamTimeout",50,1000,proposed.jamTimeout);
    proposed.rotationsPerPortion=number("rotationsPerPortion",1,255,proposed.rotationsPerPortion); proposed.maxRotations=number("maxRotations",1,255,proposed.maxRotations);
    auto flag=[&](const char* key,bool& dest) { if(doc.containsKey(key)) { if(!doc[key].is<bool>()) valid=false; else dest=doc[key].as<bool>(); } };
    flag("mqttEnabled",proposed.mqttEnabled); flag("haDiscovery",proposed.haDiscovery); flag("lightBarrierEnabled",proposed.lightBarrierEnabled); flag("endSwitchEnabled",proposed.endSwitchEnabled);
    if(proposed.rotationsPerPortion>proposed.maxRotations || !proposed.timezone[0] || !proposed.mqttPrefix[0] || strpbrk(proposed.mqttPrefix,"+# ") || (proposed.mqttEnabled && !proposed.mqttServer[0])) valid=false;
    if(doc.containsKey("adminPassword")) {
        if(!doc["adminPassword"].is<const char*>() || strlen(doc["adminPassword"])<16 || strlen(doc["adminPassword"])>128) valid=false;
        else strlcpy(proposed.adminPassword,passwordHash(doc["adminPassword"].as<String>()).c_str(),sizeof(proposed.adminPassword));
    }
    if(!valid) { error(400,"Invalid configuration"); return; }
    std::unique_ptr<FeedMateConfig> previous(new FeedMateConfig(config)); config=proposed;
    if(!saveConfig()) { config=*previous; error(503,"Config commit failed"); return; }
    if(doc.containsKey("adminPassword")) sessionToken="";
    // Network changes require an explicit restart after response delivery.
    webServer.send(200,"application/json","{\"status\":\"saved; restart device to apply network and timezone settings\"}");
}
static void logs() {
    if(!auth()) return;
    DynamicJsonDocument doc(16384); JsonArray arr=doc.createNestedArray("logs");
    for(unsigned i=0;i<logCount;i++) { const auto& e=eventLog[(logic::oldest(logIndex,logCount,MAX_LOG_ENTRIES)+i)%MAX_LOG_ENTRIES]; JsonObject o=arr.createNestedObject(); o["ts"]=e.timestamp; o["uptime"]=e.uptime; o["type"]=e.eventType; o["msg"]=e.message; }
    sendJson(doc);
}
void setupWebServer() {
#if defined(ESP32)
    const char* headers[]={"Authorization"}; webServer.collectHeaders(headers,1);
#else
    webServer.collectHeaders("Authorization");
#endif
    webServer.on("/",HTTP_GET,[](){ File f=LittleFS.open(HTML_FILE,"r"); if(!f) {error(503,"Upload LittleFS data image");return;} webServer.sendHeader("X-Content-Type-Options","nosniff"); webServer.streamFile(f,"text/html; charset=utf-8"); });
    webServer.on("/api/login",HTTP_POST,login);
    webServer.on("/api/logout",HTTP_POST,[](){if(!auth())return;sessionToken="";webServer.send(204);});
    webServer.on("/api/status",HTTP_GET,status);
    webServer.on("/api/feed",HTTP_POST,feed);
    webServer.on("/api/schedule",HTTP_GET,getSchedule); webServer.on("/api/schedule",HTTP_POST,postSchedule);
    webServer.on("/api/config",HTTP_GET,getConfig); webServer.on("/api/config",HTTP_POST,postConfig);
    webServer.on("/api/logs",HTTP_GET,logs);
    webServer.on("/api/reset-fault",HTTP_POST,[](){if(!auth())return;if(!resetFault()){error(409,"Fault input remains active");return;}webServer.send(200,"application/json","{}");});
    // No default-password OTA endpoint. Firmware updates use USB.
    webServer.onNotFound([](){error(404,"Not found");}); webServer.begin();
}
