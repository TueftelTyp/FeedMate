#pragma once
#include <ctime>
inline tm* feedmate_localtime_r(const time_t* t, tm* output) {
    return localtime_s(output,t)==0 ? output : nullptr;
}
