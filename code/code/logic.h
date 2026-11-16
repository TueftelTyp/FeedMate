#pragma once
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <math.h>
namespace logic {
inline bool headerHasToken(const char* list,const char* token) {
    if(!list||!token||!token[0]) return false;
    const size_t wanted=strlen(token);
    while(*list) {
        while(*list==' '||*list=='\t'||*list==',') ++list;
        const char* start=list;
        while(*list&&*list!=',') ++list;
        const char* end=list;
        while(end>start&&(end[-1]==' '||end[-1]=='\t')) --end;
        if(size_t(end-start)==wanted) {
            size_t i=0;
            for(;i<wanted;i++) {
                char a=start[i],b=token[i];
                if(a>='A'&&a<='Z') a=char(a-'A'+'a');
                if(b>='A'&&b<='Z') b=char(b-'A'+'a');
                if(a!=b) break;
            }
            if(i==wanted) return true;
        }
        if(*list==',') ++list;
    }
    return false;
}
inline bool durationValid(long ms) { return ms > 0 && ms <= 30000; }
inline bool due(bool active, unsigned hour, unsigned minute, unsigned second, uint32_t lastDate,
                unsigned nowHour, unsigned nowMinute, unsigned nowSecond, uint32_t date,
                bool startupMinute=false, unsigned startupSecond=0) {
    return active && hour == nowHour && minute == nowMinute && second <= nowSecond &&
        date > lastDate && (!startupMinute || second >= startupSecond);
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
