// These tests execute the production controller and scheduler with fake GPIO,
// monotonic time and persistence. No ESP hardware or copied state machine.
#define CONFIG_H
#include <cstdint>
#include <ctime>
#include <cassert>
#include <cstdio>
#include <algorithm>
#include <string>
#include <vector>
#include "../code/logic.h"
#define IRAM_ATTR
#define HIGH 1
#define LOW 0
#define STATUS_LED_ON LOW
#define STATUS_LED_OFF HIGH
#define FALLING 2
#define PIN_MOTOR_GATE 5
#define PIN_JAM_SENSOR 6
#define PIN_LIGHT_BARRIER 7
#define PIN_HALL_SENSOR 8
#define PIN_MANUAL_BUTTON 3
#define PIN_LED_BUILTIN 2
#define MOTOR_DEFAULT_DURATION_MS 3000
enum {EVT_FEEDING_STARTED,EVT_FEEDING_ABORTED,EVT_FEEDING_OK,EVT_SENSOR_LOW_FOOD,EVT_STORAGE_ERROR};
struct { bool endSwitchEnabled=false,lightBarrierEnabled=false; uint16_t jamTimeout=500; uint8_t rotationsPerPortion=10,maxRotations=50; } config;
struct FeedingSlot {bool active;uint8_t hour,minute,second;uint16_t durationMs;uint32_t lastRunDate;char foodName[33]{};float portionGrams=0;};
FeedingSlot schedule[8]{};
bool storageReady=true,logsDirty=false,clockValid=true,motorRunning=false,jamActive=false;
unsigned long motorStartTime=0,motorDuration=0,jamDetectedAt=0;
uint32_t rotationCount=0,nowMs=1000;
uint8_t foodLevelPercent=0;
int pins[16]{};
uint32_t epoch=1762326000; // replaced below with deterministic UTC tm
bool commitOk=true,logOk=true;
bool profileOk=true;
int commits=0;
FeedingSlot persisted[8]{};
std::vector<int> events;
uint32_t millis(){return nowMs;}
uint32_t micros(){return nowMs*1000;}
int digitalRead(int p){return pins[p];}
void digitalWrite(int p,int v){pins[p]=v;}
void noInterrupts(){} void interrupts(){}
void attachInterrupt(int,void(*)(),int){}
void detachInterrupt(int){}
int digitalPinToInterrupt(int p){return p;}
int constrain(int x,int lo,int hi){return std::max(lo,std::min(hi,x));}
uint32_t utcNow(){return epoch;}
bool saveSchedule(){++commits;if(commitOk)std::copy(std::begin(schedule),std::end(schedule),persisted);return commitOk;}
bool estimateFood(const char*,double,uint16_t& duration,uint8_t& target){duration=2000;target=0;return profileOk;}
void addLogEntry(uint8_t type,const char*){events.push_back(type);logsDirty=!logOk;}
bool startMotor(uint16_t);
bool requestMotor(uint16_t,uint8_t);
void stopMotor(bool success=false,const char* reason="abort");
#include "../code/sensors.cpp"
#include "../code/schedule.cpp"
static void reset(){
    std::fill(std::begin(pins),std::end(pins),0);pins[PIN_MANUAL_BUTTON]=HIGH;
    motorRunning=jamActive=logsDirty=false;storageReady=clockValid=commitOk=logOk=profileOk=true;
    config.endSwitchEnabled=false;config.lightBarrierEnabled=false;config.rotationsPerPortion=10;config.maxRotations=50;
    pendingDuration=0;nowMs=1000;events.clear();commits=0;std::fill(std::begin(schedule),std::end(schedule),FeedingSlot{});
}
static void dueSlot(int i=0){time_t t=epoch;tm local{};localtime_r(&t,&local);schedule[i]={true,uint8_t(local.tm_hour),uint8_t(local.tm_min),uint8_t(local.tm_sec),3000,0};}
int main(){
    uint16_t portionMs=0;uint8_t portionTurns=0;
    assert(logic::estimatePortion(50,100,3000,0,false,50,portionMs,portionTurns));
    assert(portionMs==1500&&portionTurns==0);
    assert(logic::estimatePortion(50,100,3000,20,true,50,portionMs,portionTurns));
    assert(portionTurns==10&&portionMs==2250);
    assert(!logic::estimatePortion(500,10,3000,20,true,50,portionMs,portionTurns));
    assert(!logic::estimatePortion(500,1,3000,0,false,50,portionMs,portionTurns));
    assert(!logic::estimatePortion(50,100,3000,0,true,50,portionMs,portionTurns));
    assert(!logic::sessionValid("","Bearer anything",1000,1000));
    assert(!logic::sessionValid("secret","Bearer anything",1000,1000));
    assert(!logic::sessionValid("secret","",1000,1000));
    assert(!logic::sessionValid("secret","Bearer secret-extra",1000,1000));
    assert(logic::sessionValid("secret","Bearer secret",1000,1000));
    assert(!logic::sessionValid("secret","Bearer secret",1801000,1000));
    assert(logic::sessionValid("secret","Bearer secret",84,0xfffffff0U));
    reset(); assert(!startMotor(0));assert(!startMotor(30001));assert(startMotor(3000));
    const auto start=motorStartTime;nowMs+=100;assert(!startMotor(2000));assert(motorStartTime==start);
    nowMs=start+3000;serviceMotor();assert(!motorRunning&&pins[PIN_MOTOR_GATE]==LOW&&events.back()==EVT_FEEDING_OK);
    reset();assert(startMotor(3000));pins[PIN_JAM_SENSOR]=HIGH;serviceMotor();nowMs+=500;serviceMotor();
    assert(jamActive&&!motorRunning&&events.back()==EVT_FEEDING_ABORTED);assert(!resetFault());pins[PIN_JAM_SENSOR]=LOW;assert(resetFault());assert(!motorRunning);
    reset();pins[PIN_JAM_SENSOR]=HIGH;assert(!startMotor(3000));
    reset();config.endSwitchEnabled=true;assert(startMotor(3000));nowMs+=1500;serviceMotor();assert(jamActive&&!motorRunning);
    reset();config.endSwitchEnabled=true;assert(startMotor(3000));hallEdges=10;serviceMotor();assert(!motorRunning&&!jamActive&&events.back()==EVT_FEEDING_OK);
    reset();config.endSwitchEnabled=true;assert(startMotor(3000));hallEdges=51;serviceMotor();assert(jamActive&&!motorRunning);
    reset();config.endSwitchEnabled=true;assert(startMotor(1000));nowMs+=1000;serviceMotor();assert(jamActive&&events.back()==EVT_FEEDING_ABORTED);
    reset();storageReady=false;assert(!startMotor(3000));reset();logOk=false;assert(!startMotor(3000));assert(pins[PIN_MOTOR_GATE]==LOW);
    reset();assert(requestMotor(3000));assert(!motorRunning);assert(!requestMotor(3000));serviceMotorRequest();assert(motorRunning);
    reset();assert(requestMotor(3000));pins[PIN_JAM_SENSOR]=HIGH;serviceMotorRequest();assert(!motorRunning);
    reset();dueSlot();clockValid=false;checkSchedule();assert(!motorRunning&&commits==0);
    reset();dueSlot();commitOk=false;checkSchedule();assert(!motorRunning&&!storageReady);
    reset();dueSlot();checkSchedule();assert(motorRunning&&commits==1&&persisted[0].lastRunDate>0);
    // Reboot in the same scheduled minute: reload the committed reservation.
    motorRunning=false;std::copy(std::begin(persisted),std::end(persisted),schedule);checkSchedule();assert(!motorRunning&&commits==1);
    epoch+=86400;checkSchedule();assert(motorRunning&&commits==2);
    motorRunning=false;epoch-=86400;checkSchedule();assert(!motorRunning&&commits==2);
    reset();dueSlot(0);dueSlot(1);checkSchedule();auto deadline=motorStartTime;checkSchedule();assert(commits==1&&motorStartTime==deadline);
    nowMs+=3000;serviceMotor();checkSchedule();assert(motorRunning&&commits==2);
    reset();dueSlot();strcpy(schedule[0].foodName,"Testfutter");schedule[0].portionGrams=25;
    checkSchedule();assert(commits==1&&!motorRunning);serviceMotorRequest();assert(motorRunning&&motorDuration==2000);
    reset();dueSlot();strcpy(schedule[0].foodName,"Fehlend");schedule[0].portionGrams=25;profileOk=false;
    checkSchedule();assert(commits==1&&!motorRunning&&events.back()==EVT_FEEDING_ABORTED);
    reset();nowMs=0xfffffff0U;assert(startMotor(100));nowMs=84;serviceMotor();assert(!motorRunning);
    assert(logic::oldest(3,3,100)==0);assert(logic::oldest(3,100,100)==3);assert(logic::oldest(0,100,100)==0);
    assert(logic::due(true,7,0,5,20261231,7,0,5,20270101));assert(!logic::due(true,7,0,5,20261105,7,0,5,20261105));
    reset();config.lightBarrierEnabled=true;checkSensors();nowMs+=500;checkSensors();assert(foodLevelPercent==0);
    pins[PIN_LIGHT_BARRIER]=HIGH;checkSensors();nowMs+=500;checkSensors();assert(foodLevelPercent==100);
    pins[PIN_LIGHT_BARRIER]=LOW;checkSensors();nowMs+=500;checkSensors();assert(events.back()==EVT_SENSOR_LOW_FOOD);
    puts("PASS: controller, scheduler, persistence-failure, reboot, sensors, ring index and timer rollover assertions");
}
