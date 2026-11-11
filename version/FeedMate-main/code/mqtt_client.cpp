#include "config.h"
static uint32_t retryAt=0,retryDelay=5000,lastSequence=0;
static bool configured=false;
static String prefix;
static bool publish(const String& suffix,const char* payload,bool retained=false) {
    bool ok=mqttClient.publish((prefix+suffix).c_str(),payload,retained);
    if(!ok) Serial.println("MQTT publish failed");
    return ok;
}
static void callback(char* topic,byte* payload,unsigned int length) {
    if(String(topic)!=prefix+"/control/feed" || length>384) return;
    StaticJsonDocument<512> doc;
    if(deserializeJson(doc,payload,length)) return;
    // Retained/stale commands from a prior boot cannot cause a feeding.
    // A sequence is consumed even if the motor rejects the request.
    if(!doc["boot"].is<const char*>() || doc["boot"].as<String>()!=bootNonce || !doc["seq"].is<uint32_t>() || !doc["duration"].is<long>()) return;
    uint32_t sequence=doc["seq"].as<uint32_t>();
    if(sequence<=lastSequence || !logic::durationValid(doc["duration"].as<long>())) return;
    lastSequence=sequence;
    bool started=requestMotor(doc["duration"].as<uint16_t>());
    // Defer network replies until the motor is off.
    Serial.println(started?"MQTT feed accepted":"MQTT feed rejected");
}
static void discovery() {
    if(!config.haDiscovery) return;
    StaticJsonDocument<768> doc;
    String id=String("feedmate_")+deviceId();
    doc["name"]="FeedMate food present"; doc["unique_id"]=id+"_food";
    doc["state_topic"]=prefix+"/sensor/food_present";
    doc["availability_topic"]=prefix+"/status/online"; doc["payload_available"]="1";doc["payload_not_available"]="0";
    doc["payload_on"]="1";doc["payload_off"]="0";
    String json;serializeJson(doc,json);
    String topic="homeassistant/binary_sensor/"+id+"/food/config";
    if(!mqttClient.publish(topic.c_str(),config.lightBarrierEnabled?json.c_str():"",true)) Serial.println("Discovery publish failed");
}
void publishMQTTStatus() {
    if(!mqttClient.connected() || motorRunning) return;
    publish("/status/online","1",true);
    publish("/status/boot",bootNonce.c_str(),true);
    publish("/status/motor_running",motorRunning?"1":"0");
    publish("/status/jam_active",jamActive?"1":"0",true);
    publish("/status/clock_valid",clockValid?"1":"0",true);
    publish("/status/power",powerOnline?"1":"0",true);
    if(config.lightBarrierEnabled) publish("/sensor/food_present",foodLevelPercent==100?"1":"0",true);
    if(config.endSwitchEnabled) publish("/sensor/rotations",String(rotationCount).c_str());
    publish("/status/wifi_rssi",String(WiFi.RSSI()).c_str());
}
void setupMQTT() {
    prefix=String(config.mqttPrefix)+"/"+deviceId();
    configured=config.mqttEnabled && config.mqttServer[0] && mqttClient.setBufferSize(1024);
    mqttClient.setServer(config.mqttServer,config.mqttPort); mqttClient.setCallback(callback);
    mqttClient.setKeepAlive(FEEDMATE_KEEPALIVE); mqttClient.setSocketTimeout(1);
}
void serviceMQTT() {
    if(!configured || motorRunning || WiFi.status()!=WL_CONNECTED) return;
    if(!mqttClient.connected()) {
        if(int32_t(millis()-retryAt)<0) return;
        String id=String("feedmate-")+deviceId();
        String will=prefix+"/status/online";
        bool ok=mqttClient.connect(id.c_str(),config.mqttUser[0]?config.mqttUser:nullptr,config.mqttUser[0]?config.mqttPassword:nullptr,will.c_str(),1,true,"0");
        retryAt=millis()+retryDelay; retryDelay=min(retryDelay*2,60000U);
        if(!ok) { Serial.printf("MQTT connection failed: %d\n",mqttClient.state()); return; }
        if(!mqttClient.subscribe((prefix+"/control/feed").c_str(),1)) {mqttClient.disconnect();return;}
        retryDelay=5000; discovery(); publishMQTTStatus();
    }
    mqttClient.loop();
    if(!motorRunning && millis()-lastMqttPublish>=30000) {lastMqttPublish=millis();publishMQTTStatus();}
}
