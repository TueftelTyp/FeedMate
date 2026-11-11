#include "config.h"

void checkSchedule() {
    DateTime now = rtc.now();
    uint8_t currentHour = now.hour();
    uint8_t currentMinute = now.minute();

    // Reset daily flags at midnight
    static uint8_t lastDay = 0;
    if (now.day() != lastDay) {
        for (int i = 0; i < 8; i++) {
            schedule[i].lastRunToday = false;
        }
        lastDay = now.day();
    }

    for (int i = 0; i < 8; i++) {
        if (!schedule[i].active) continue;
        if (schedule[i].lastRunToday) continue;

        if (currentHour == schedule[i].hour && currentMinute == schedule[i].minute) {
            Serial.printf("Scheduled feeding #%d at %02d:%02d\n", i, currentHour, currentMinute);
            startMotor(schedule[i].durationMs);
            schedule[i].lastRunToday = true;
            saveSchedule();
            addLogEntry(EVT_FEEDING_OK, "Scheduled feeding executed");
        }
    }
}