#pragma once
#include <Arduino.h>
#if defined(ESP32)
#include <WiFi.h>
#include <WebServer.h>
using FeedMateWebServer = WebServer;
// Classic ESP32-WROOM / DevKit V1. C3/S3 require a separate pin profile.
#define PIN_MOTOR_GATE 25
#define PIN_JAM_SENSOR 26
#define PIN_LIGHT_BARRIER 27
#define PIN_HALL_SENSOR 33
#define PIN_MANUAL_BUTTON 32
#define PIN_RTC_SDA 21
#define PIN_RTC_SCL 22
#define PIN_POWER_SENSE 34  // ADC1: usable while WiFi is active, input only
#define PIN_LED_BUILTIN 2
#define STATUS_LED_ON HIGH
#define STATUS_LED_OFF LOW
inline String deviceId() {
    uint64_t mac=ESP.getEfuseMac();
    char id[13]; snprintf(id,sizeof(id),"%04x%08x",unsigned(mac>>32),unsigned(mac));
    return String(id);
}
inline void platformIdle() { delay(1); } // Let FreeRTOS idle/WiFi tasks run.
inline const char* platformName() { return "ESP32"; }
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
using FeedMateWebServer = ESP8266WebServer;
#define PIN_MOTOR_GATE D5
#define PIN_JAM_SENSOR D6
#define PIN_LIGHT_BARRIER D7
#define PIN_HALL_SENSOR D8
#define PIN_MANUAL_BUTTON D3
#define PIN_RTC_SDA D2
#define PIN_RTC_SCL D1
#define PIN_POWER_SENSE A0
#define PIN_LED_BUILTIN LED_BUILTIN
#define STATUS_LED_ON LOW
#define STATUS_LED_OFF HIGH
inline String deviceId() { return String(ESP.getChipId(),HEX); }
inline void platformIdle() { yield(); }
inline const char* platformName() { return "ESP8266"; }
#else
#error Unsupported board: choose esp32dev or d1_mini in platformio.ini
#endif
