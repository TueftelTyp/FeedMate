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
static String recoveryCode;
static uint8_t recoveryFailures=0;
static uint32_t lastRecoveryAttempt=0;
static void error(int status,const char* message);
static bool body(JsonDocument& doc);
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
static String secretHash(const char* scope,const String& secret) {
    // Device-specific salt; high-entropy generated password is the initial credential.
    String input=String("FeedMate:")+scope+":"+deviceId()+":"+secret;
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
static String passwordHash(const String& password) { return secretHash("admin",password); }
static String fleetHash(const String& key) { return secretHash("fleet",key); }
static bool same(const String& a,const String& b) {
    return logic::secureEqual(a.c_str(),b.c_str());
}
void setupAuth() {
    recoveryCode=randomHex(16);
    Serial.printf("RECOVERY CODE FOR THIS BOOT (USB/Serial only): %s\n",recoveryCode.c_str());
    // Old placeholder hashes cannot be migrated securely.
    if(strlen(config.adminPassword)!=64) {
        String password=randomHex(16);
        strlcpy(config.adminPassword,passwordHash(password).c_str(),sizeof(config.adminPassword));
        if(!saveConfig()) { config.adminPassword[0]=0; storageReady=false; return; }
        Serial.printf("NEW ADMIN PASSWORD (store securely): %s\n",password.c_str());
    }
}
static void recover() {
    if(!recoveryCode.length() || recoveryFailures>=5 ||
       (lastRecoveryAttempt && millis()-lastRecoveryAttempt<2000)) { error(429,"Recovery unavailable or rate limited"); return; }
    lastRecoveryAttempt=millis();
    StaticJsonDocument<512> doc;
    if(!body(doc)) return;
    if(!doc["code"].is<const char*>() || !doc["newPassword"].is<const char*>() ||
       strlen(doc["newPassword"])<16 || strlen(doc["newPassword"])>128) { error(400,"Invalid recovery request"); return; }
    if(!same(doc["code"].as<String>(),recoveryCode)) { ++recoveryFailures; error(401,"Invalid recovery code"); return; }
    char previous[sizeof(config.adminPassword)];
    strlcpy(previous,config.adminPassword,sizeof(previous));
    strlcpy(config.adminPassword,passwordHash(doc["newPassword"].as<String>()).c_str(),sizeof(config.adminPassword));
    if(!saveConfig()) { strlcpy(config.adminPassword,previous,sizeof(config.adminPassword)); error(503,"Password commit failed"); return; }
    sessionToken=""; recoveryCode="";
    webServer.send(200,"application/json","{\"status\":\"Password changed; log in again\"}");
}
static void cors() {
    webServer.sendHeader("Access-Control-Allow-Origin","*");
    webServer.sendHeader("Access-Control-Allow-Headers","Authorization, Content-Type, X-FeedMate-Key");
    webServer.sendHeader("Access-Control-Allow-Methods","GET, POST, OPTIONS");
}
static void error(int status,const char* message) { cors(); webServer.send(status,"application/json",String("{\"error\":\"")+message+"\"}"); }
static bool auth() {
    cors(); webServer.sendHeader("Cache-Control","no-store");
    bool session=logic::sessionValid(sessionToken.c_str(),webServer.header("Authorization").c_str(),millis(),sessionStart);
    String fleetKey=webServer.header("X-FeedMate-Key");
    bool fleet=config.fleetKeyHash[0] && fleetKey.length()>=16 && same(fleetHash(fleetKey),config.fleetKeyHash);
    if(!session && !fleet) { error(401,"Unauthorized"); return false; }
    return true;
}
static void sendJson(const JsonDocument& doc) {
    if(doc.overflowed()) { error(500,"Response too large"); return; }
    String body; serializeJson(doc,body); cors(); webServer.sendHeader("Cache-Control","no-store"); webServer.send(200,"application/json",body);
}
static bool body(JsonDocument& doc) {
    if(webServer.arg("plain").length()>16384 || deserializeJson(doc,webServer.arg("plain")) || !doc.is<JsonObject>()) { error(400,"Invalid JSON object"); return false; }
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
    doc["deviceName"]=config.deviceName;
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
    DynamicJsonDocument doc(12288); JsonArray slots=doc.createNestedArray("slots");
    for(const auto& s:schedule) { JsonObject o=slots.createNestedObject(); o["active"]=s.active; o["hour"]=s.hour; o["minute"]=s.minute; o["second"]=s.second; o["duration"]=s.durationMs; o["lastRunDate"]=s.lastRunDate; o["foodName"]=s.foodName; o["portionGrams"]=s.portionGrams; }
    sendJson(doc);
}
static void postSchedule() {
    if(!auth()) return;
    DynamicJsonDocument doc(12288); if(!body(doc)) return;
    if(!doc["slots"].is<JsonArray>() || doc["slots"].size()>MAX_SCHEDULE_ENTRIES) { error(400,"Maximum 32 slots"); return; }
    FeedingSlot proposed[MAX_SCHEDULE_ENTRIES]{};
    for(int i=0;i<MAX_SCHEDULE_ENTRIES;i++) proposed[i].durationMs=MOTOR_DEFAULT_DURATION_MS;
    for(size_t i=0;i<doc["slots"].size();i++) {
        JsonObject o=doc["slots"][i];
        if(!o["active"].is<bool>() || !o["hour"].is<int>() || !o["minute"].is<int>() ||
           o["hour"].as<int>()<0 || o["hour"].as<int>()>23 || o["minute"].as<int>()<0 || o["minute"].as<int>()>59 ||
           (o.containsKey("second") && (!o["second"].is<int>() || o["second"].as<int>()<0 || o["second"].as<int>()>59)) ||
           (o.containsKey("duration") && (!o["duration"].is<long>() || !logic::durationValid(o["duration"].as<long>())))) { error(400,"Invalid slot"); return; }
        proposed[i].active=o["active"]; proposed[i].hour=o["hour"]; proposed[i].minute=o["minute"]; proposed[i].second=o["second"] | 0;
        if(o.containsKey("duration")) proposed[i].durationMs=o["duration"];
        if(o.containsKey("foodName")) {
            if(!o["foodName"].is<const char*>() || strlen(o["foodName"])>32 ||
               (!o["portionGrams"].is<double>() && !o["portionGrams"].is<long>())) {error(400,"Invalid portion slot");return;}
            strlcpy(proposed[i].foodName,o["foodName"],sizeof(proposed[i].foodName));
            proposed[i].portionGrams=o["portionGrams"].as<float>();
            if(proposed[i].foodName[0] && (!isfinite(proposed[i].portionGrams) || proposed[i].portionGrams<=0 || proposed[i].portionGrams>500)) {error(400,"Invalid portion grams");return;}
        }
        if(proposed[i].active && !proposed[i].foodName[0]) {error(400,"Active slots require a food profile");return;}
        if(proposed[i].active && proposed[i].foodName[0]) {
            uint16_t duration;uint8_t target;
            if(!estimateFood(proposed[i].foodName,proposed[i].portionGrams,duration,target)) {error(400,"Scheduled portion cannot be delivered");return;}
        }
        proposed[i].lastRunDate=schedule[i].lastRunDate;
    }
    FeedingSlot previous[MAX_SCHEDULE_ENTRIES]; memcpy(previous,schedule,sizeof(schedule)); memcpy(schedule,proposed,sizeof(schedule));
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
    doc["deviceName"]=config.deviceName; doc["fleetEnabled"]=config.fleetKeyHash[0]!=0;
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
    stringField("deviceName",proposed.deviceName,sizeof(proposed.deviceName));
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
    if(doc.containsKey("fleetKey")) {
        if(!doc["fleetKey"].is<const char*>() || strlen(doc["fleetKey"])<16 || strlen(doc["fleetKey"])>128) valid=false;
        else strlcpy(proposed.fleetKeyHash,fleetHash(doc["fleetKey"].as<String>()).c_str(),sizeof(proposed.fleetKeyHash));
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
static bool readFoods(DynamicJsonDocument& doc) {
    File file=LittleFS.open(FOODS_FILE,"r");
    if(!file) { doc.createNestedArray("foods"); return true; }
    return !deserializeJson(doc,file) && doc["foods"].is<JsonArray>();
}
bool estimateFood(const char* name,double grams,uint16_t& duration,uint8_t& target) {
    DynamicJsonDocument doc(4096);
    if(!readFoods(doc)) return false;
    for(JsonObject item:doc["foods"].as<JsonArray>()) {
        if(strcmp(item["name"] | "",name)) continue;
        bool useTurns=!strcmp(item["mode"] | "", "rotations");
        if(useTurns && !config.endSwitchEnabled) return false;
        return logic::estimatePortion(grams,item["sampleGrams"].as<double>(),item["sampleDurationMs"].as<uint32_t>(),
             item["sampleRotations"].as<uint8_t>(),useTurns,config.maxRotations,duration,target);
    }
    return false;
}
static void getFoods() {
    if(!auth()) return;
    DynamicJsonDocument doc(4096);
    if(!readFoods(doc)) { error(503,"Food profiles unreadable"); return; }
    sendJson(doc);
}
static void postFoods() {
    if(!auth()) return;
    DynamicJsonDocument doc(4096); if(!body(doc)) return;
    if(!doc["foods"].is<JsonArray>() || doc["foods"].size()>8) { error(400,"Maximum eight food profiles"); return; }
    for(JsonObject item:doc["foods"].as<JsonArray>()) {
        if(!item["name"].is<const char*>() || !strlen(item["name"]) || strlen(item["name"])>32 ||
           !item["mode"].is<const char*>() ||
           (strcmp(item["mode"],"time") && strcmp(item["mode"],"rotations")) ||
           (!item["sampleGrams"].is<double>() && !item["sampleGrams"].is<long>()) ||
           !item["sampleDurationMs"].is<long>() || !item["sampleRotations"].is<long>()) {
            error(400,"Invalid food profile"); return;
        }
        double grams=item["sampleGrams"].as<double>();
        long ms=item["sampleDurationMs"].as<long>(),rotations=item["sampleRotations"].as<long>();
        if(!isfinite(grams) || grams<=0 || grams>1000 || ms<1 || ms>30000 || rotations<0 || rotations>255 ||
           (!strcmp(item["mode"],"rotations") && !rotations)) { error(400,"Invalid calibration values"); return; }
    }
    for(size_t i=0;i<doc["foods"].size();++i)
        for(size_t j=i+1;j<doc["foods"].size();++j)
            if(!strcmp(doc["foods"][i]["name"],doc["foods"][j]["name"])) {error(400,"Food names must be unique");return;}
    for(const auto& slot:schedule) if(slot.active && slot.foodName[0]) {
        bool found=false;
        for(JsonObject item:doc["foods"].as<JsonArray>()) if(!strcmp(slot.foodName,item["name"])) found=true;
        if(!found) {error(409,"Food profile is used by an active schedule");return;}
    }
    if(!atomicJson(FOODS_FILE,doc)) { error(503,"Food profile save failed"); return; }
    sendJson(doc);
}
static void feedPortion() {
    if(!auth()) return;
    StaticJsonDocument<256> request; if(!body(request)) return;
    if((!request["index"].is<int>() && !request["foodName"].is<const char*>()) ||
       (!request["grams"].is<double>() && !request["grams"].is<long>())) { error(400,"Food and grams required"); return; }
    int index=request["index"] | -1; double grams=request["grams"].as<double>();
    if(!isfinite(grams) || grams<=0 || grams>500) { error(400,"Invalid portion"); return; }
    DynamicJsonDocument profiles(4096);
    if(!readFoods(profiles)) { error(503,"Food profiles unreadable"); return; }
    if(request["foodName"].is<const char*>()) {
        index=-1;
        for(size_t i=0;i<profiles["foods"].size();++i)
            if(!strcmp(profiles["foods"][i]["name"] | "",request["foodName"])) {index=int(i);break;}
    }
    if(index<0 || size_t(index)>=profiles["foods"].size()) { error(404,"Food profile not found"); return; }
    JsonObject item=profiles["foods"][index];
    bool useTurns=!strcmp(item["mode"],"rotations");
    if(useTurns && !config.endSwitchEnabled) { error(409,"Hall sensor must be enabled"); return; }
    uint8_t target=0; uint16_t limit=0;
    if(!logic::estimatePortion(grams,item["sampleGrams"].as<double>(),item["sampleDurationMs"].as<uint32_t>(),
         item["sampleRotations"].as<uint8_t>(),useTurns,config.maxRotations,limit,target)) {
        error(400,"Portion exceeds safety limits"); return;
    }
    if(!requestMotor(limit,target)) { error(409,"Motor busy, fault or storage unavailable"); return; }
    StaticJsonDocument<256> response;
    response["status"]="queued"; response["estimatedGrams"]=grams;
    response["durationLimitMs"]=limit; response["targetRotations"]=target;
    String json; serializeJson(response,json); webServer.send(202,"application/json",json);
}
static void exportBackup() {
    if(!auth()) return;
    DynamicJsonDocument section(8192);
    String backup=String("{\"format\":\"FeedMate-v1.2\",\"deviceId\":\"")+deviceId()+"\"";
    for(const char* path:{CONFIG_FILE,SCHEDULE_FILE,FOODS_FILE}) {
        File file=LittleFS.open(path,"r");
        section.clear();
        if(!file) { if(strcmp(path,FOODS_FILE)) {error(503,"Backup source missing");return;} section.createNestedArray("foods"); }
        else if(deserializeJson(section,file)) {error(503,"Backup source unreadable");return;}
        const char* key=!strcmp(path,CONFIG_FILE)?"config":!strcmp(path,SCHEDULE_FILE)?"schedule":"foodProfiles";
        backup+=String(",\"")+key+"\":";
        serializeJson(section,backup);
        if(backup.length()>12000) {error(503,"Backup too large");return;}
    }
    backup+='}';
    webServer.sendHeader("Content-Disposition","attachment; filename=feedmate-backup.json");
    webServer.sendHeader("Cache-Control","no-store");
    webServer.send(200,"application/json",backup);
}
void setupWebServer() {
#if defined(ESP32)
    const char* headers[]={"Authorization","X-FeedMate-Key"}; webServer.collectHeaders(headers,2);
#else
    webServer.collectHeaders("Authorization","X-FeedMate-Key");
#endif
    webServer.onNotFound([](){
        if(webServer.method()==HTTP_OPTIONS) {cors();webServer.send(204);return;}
        error(404,"Not found");
    });
    webServer.on("/",HTTP_GET,[](){ File f=LittleFS.open(HTML_FILE,"r"); if(!f) {error(503,"Upload LittleFS data image");return;} webServer.sendHeader("X-Content-Type-Options","nosniff"); webServer.streamFile(f,"text/html; charset=utf-8"); });
    webServer.on("/api/login",HTTP_POST,login);
    webServer.on("/api/recover",HTTP_POST,recover);
    webServer.on("/api/logout",HTTP_POST,[](){if(!auth())return;sessionToken="";webServer.send(204);});
    webServer.on("/api/status",HTTP_GET,status);
    webServer.on("/api/feed",HTTP_POST,feed);
    webServer.on("/api/schedule",HTTP_GET,getSchedule); webServer.on("/api/schedule",HTTP_POST,postSchedule);
    webServer.on("/api/config",HTTP_GET,getConfig); webServer.on("/api/config",HTTP_POST,postConfig);
    webServer.on("/api/logs",HTTP_GET,logs);
    webServer.on("/api/foods",HTTP_GET,getFoods); webServer.on("/api/foods",HTTP_POST,postFoods);
    webServer.on("/api/portion",HTTP_POST,feedPortion);
    webServer.on("/api/backup",HTTP_GET,exportBackup);
    webServer.on("/api/reset-fault",HTTP_POST,[](){if(!auth())return;if(!resetFault()){error(409,"Fault input remains active");return;}webServer.send(200,"application/json","{}");});
    // No default-password OTA endpoint. Firmware updates use USB.
    webServer.begin();
}
