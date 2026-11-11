#pragma once
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <math.h>
namespace logic {
inline bool durationValid(long ms) { return ms > 0 && ms <= 30000; }
inline bool due(bool active, unsigned hour, unsigned minute, unsigned second, uint32_t lastDate,
                unsigned nowHour, unsigned nowMinute, unsigned nowSecond, uint32_t date) {
    return active && hour == nowHour && minute == nowMinute && second == nowSecond && date > lastDate;
}
inline unsigned oldest(unsigned next, unsigned count, unsigned capacity) {
    return (next + capacity - count) % capacity;
}
inline bool expired(uint32_t now, uint32_t start, uint32_t duration) {
    return uint32_t(now - start) >= duration;
}
inline bool secureEqual(const char* a, const char* b) {
    size_t n=strlen(a);
    if(n!=strlen(b)) return false;
    uint8_t difference=0;
    for(size_t i=0;i<n;i++) difference|=uint8_t(a[i])^uint8_t(b[i]);
    return difference==0;
}
inline bool sessionValid(const char* token,const char* authorization,uint32_t now,uint32_t start) {
    return token[0] && !expired(now,start,1800000UL) &&
        strncmp(authorization,"Bearer ",7)==0 && secureEqual(token,authorization+7);
}
inline bool estimatePortion(double desiredGrams,double sampleGrams,uint32_t sampleMs,
                            uint8_t sampleTurns,bool useTurns,uint8_t maxTurns,
                            uint16_t& durationMs,uint8_t& targetTurns) {
    if(!isfinite(desiredGrams)||!isfinite(sampleGrams)||desiredGrams<=0||desiredGrams>500||
       sampleGrams<=0||sampleGrams>1000||sampleMs<1||sampleMs>30000) return false;
    double factor=desiredGrams/sampleGrams,limit;
    targetTurns=0;
    if(useTurns) {
        if(!sampleTurns) return false;
        double turns=ceil(factor*sampleTurns);
        if(turns<1||turns>maxTurns||turns>=255) return false;
        targetTurns=uint8_t(turns);
        limit=ceil(1.5*sampleMs*turns/sampleTurns);
        if(limit<1500) limit=1500;
    } else limit=ceil(factor*sampleMs);
    if(limit<1||limit>30000) return false;
    durationMs=uint16_t(limit);
    return true;
}
}
