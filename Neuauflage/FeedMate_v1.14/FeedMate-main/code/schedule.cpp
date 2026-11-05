#include "config.h"
void checkSchedule() {
    if (!clockValid || !storageReady || motorRunning) return;
    time_t epoch=utcNow();
    struct tm local;
    localtime_r(&epoch,&local);
    uint32_t date=(local.tm_year+1900)*10000UL+(local.tm_mon+1)*100+local.tm_mday;
    for(auto& slot:schedule) {
        if(!logic::due(slot.active,slot.hour,slot.minute,slot.second,slot.lastRunDate,local.tm_hour,local.tm_min,local.tm_sec,date)) continue;
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
