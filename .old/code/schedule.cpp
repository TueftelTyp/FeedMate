#include "config.h"
static bool schedulerInitialized=false;
static uint32_t schedulerStartDate=0;
static uint8_t schedulerStartHour=0,schedulerStartMinute=0,schedulerStartSecond=0;
void checkSchedule() {
    if (!clockValid || !storageReady || motorRunning) return;
    time_t epoch=utcNow();
    struct tm local;
    localtime_r(&epoch,&local);
    uint32_t date=(local.tm_year+1900)*10000UL+(local.tm_mon+1)*100+local.tm_mday;
    if(!schedulerInitialized) {
        schedulerInitialized=true; schedulerStartDate=date;
        schedulerStartHour=uint8_t(local.tm_hour); schedulerStartMinute=uint8_t(local.tm_min);
        schedulerStartSecond=uint8_t(local.tm_sec);
    }
    const bool startupMinute=date==schedulerStartDate && local.tm_hour==schedulerStartHour &&
        local.tm_min==schedulerStartMinute;
    for(auto& slot:schedule) {
        if(!logic::due(slot.active,slot.hour,slot.minute,slot.second,slot.lastRunDate,
                       local.tm_hour,local.tm_min,local.tm_sec,date,startupMinute,
                       schedulerStartSecond)) continue;
        // At-most-once: commit the reservation before any actuator action.
        slot.lastRunDate=date;
        if(!saveSchedule()) {
            storageReady=false;
            addLogEntry(EVT_STORAGE_ERROR,"Schedule reservation failed; feeding disabled");
            return;
        }
        if(slot.foodName[0]) {
            uint16_t duration=0;uint8_t target=0;
            if(!estimateFood(slot.foodName,slot.portionGrams,duration,target) || !requestMotor(duration,target))
                addLogEntry(EVT_FEEDING_ABORTED,"Scheduled portion unavailable; no automatic retry");
        } else if(!startMotor(slot.durationMs)) addLogEntry(EVT_FEEDING_ABORTED,"Scheduled feed blocked; no automatic retry");
        return;
    }
}
