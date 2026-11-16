#pragma once
#include <stdint.h>
#include <stdio.h>

namespace identity {
inline void formatMac(const uint8_t mac[6], char out[13]) {
    snprintf(out,13,"%02x%02x%02x%02x%02x%02x",
             mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
}
inline void formatLegacyEfuse(uint64_t mac, char out[13]) {
    snprintf(out,13,"%04x%08x",unsigned(mac>>32),unsigned(mac));
}
}
