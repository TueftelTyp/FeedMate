#pragma once
#include <stdint.h>
#include <stddef.h>
#include <string.h>
namespace logic {
inline bool durationValid(long ms) { return ms > 0 && ms <= 30000; }
inline bool due(bool active, unsigned hour, unsigned minute, uint32_t lastDate,
                unsigned nowHour, unsigned nowMinute, uint32_t date) {
    return active && hour == nowHour && minute == nowMinute && date > lastDate;
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
}
