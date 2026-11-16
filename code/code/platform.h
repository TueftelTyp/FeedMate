#pragma once
#include <Arduino.h>
#include "identity.h"

#if defined(ESP32)
#include <WiFi.h>
#include <WebServer.h>
#include <esp_mac.h>
using FeedMateWebServer = WebServer;

// All values can be overridden with -DFEEDMATE_PIN_* build flags. Defaults
// target Espressif reference DevKits and avoid native USB, flash/PSRAM and
// boot-strapping pins where the available GPIO count permits.
#if CONFIG_IDF_TARGET_ESP32C3
  #define FEEDMATE_DEFAULT_MOTOR_GATE     10
  #define FEEDMATE_DEFAULT_JAM_SENSOR      4
  #define FEEDMATE_DEFAULT_LIGHT_BARRIER   5
  #define FEEDMATE_DEFAULT_HALL_SENSOR      6
  #define FEEDMATE_DEFAULT_MANUAL_BUTTON    7
  #define FEEDMATE_DEFAULT_RTC_SDA          0
  #define FEEDMATE_DEFAULT_RTC_SCL          1
  #define FEEDMATE_DEFAULT_POWER_SENSE      3
  #define FEEDMATE_DEFAULT_STATUS_LED      -1
  #define FEEDMATE_PLATFORM_NAME      "ESP32-C3"
#elif CONFIG_IDF_TARGET_ESP32C6
  #define FEEDMATE_DEFAULT_MOTOR_GATE     10
  #define FEEDMATE_DEFAULT_JAM_SENSOR     11
  #define FEEDMATE_DEFAULT_LIGHT_BARRIER  18
  #define FEEDMATE_DEFAULT_HALL_SENSOR     19
  #define FEEDMATE_DEFAULT_MANUAL_BUTTON   20
  #define FEEDMATE_DEFAULT_RTC_SDA          6
  #define FEEDMATE_DEFAULT_RTC_SCL          7
  #define FEEDMATE_DEFAULT_POWER_SENSE      2
  #define FEEDMATE_DEFAULT_STATUS_LED      -1
  #define FEEDMATE_PLATFORM_NAME      "ESP32-C6"
#elif CONFIG_IDF_TARGET_ESP32S2
  #define FEEDMATE_DEFAULT_MOTOR_GATE     14
  #define FEEDMATE_DEFAULT_JAM_SENSOR     15
  #define FEEDMATE_DEFAULT_LIGHT_BARRIER  16
  #define FEEDMATE_DEFAULT_HALL_SENSOR     17
  #define FEEDMATE_DEFAULT_MANUAL_BUTTON   21
  #define FEEDMATE_DEFAULT_RTC_SDA          8
  #define FEEDMATE_DEFAULT_RTC_SCL          9
  #define FEEDMATE_DEFAULT_POWER_SENSE      1
  #define FEEDMATE_DEFAULT_STATUS_LED      -1
  #define FEEDMATE_PLATFORM_NAME      "ESP32-S2"
#elif CONFIG_IDF_TARGET_ESP32S3
  #define FEEDMATE_DEFAULT_MOTOR_GATE      4
  #define FEEDMATE_DEFAULT_JAM_SENSOR      5
  #define FEEDMATE_DEFAULT_LIGHT_BARRIER   6
  #define FEEDMATE_DEFAULT_HALL_SENSOR      7
  #define FEEDMATE_DEFAULT_MANUAL_BUTTON   15
  #define FEEDMATE_DEFAULT_RTC_SDA          8
  #define FEEDMATE_DEFAULT_RTC_SCL          9
  #define FEEDMATE_DEFAULT_POWER_SENSE      1
  #define FEEDMATE_DEFAULT_STATUS_LED      -1
  #define FEEDMATE_PLATFORM_NAME      "ESP32-S3"
#elif CONFIG_IDF_TARGET_ESP32
  #define FEEDMATE_DEFAULT_MOTOR_GATE     25
  #define FEEDMATE_DEFAULT_JAM_SENSOR     26
  #define FEEDMATE_DEFAULT_LIGHT_BARRIER  27
  #define FEEDMATE_DEFAULT_HALL_SENSOR     33
  #define FEEDMATE_DEFAULT_MANUAL_BUTTON   32
  #define FEEDMATE_DEFAULT_RTC_SDA         21
  #define FEEDMATE_DEFAULT_RTC_SCL         22
  #define FEEDMATE_DEFAULT_POWER_SENSE     34
  #define FEEDMATE_DEFAULT_STATUS_LED       2
  #define FEEDMATE_PLATFORM_NAME        "ESP32"
#elif defined(FEEDMATE_CUSTOM_PIN_PROFILE) && defined(FEEDMATE_PIN_MOTOR_GATE) && \
      defined(FEEDMATE_PIN_JAM_SENSOR) && defined(FEEDMATE_PIN_LIGHT_BARRIER) && \
      defined(FEEDMATE_PIN_HALL_SENSOR) && defined(FEEDMATE_PIN_MANUAL_BUTTON) && \
      defined(FEEDMATE_PIN_RTC_SDA) && defined(FEEDMATE_PIN_RTC_SCL) && \
      defined(FEEDMATE_PIN_POWER_SENSE)
  #define FEEDMATE_DEFAULT_STATUS_LED      -1
  #define FEEDMATE_PLATFORM_NAME "ESP32-Custom"
#else
  #error Unsupported ESP32 target. Define FEEDMATE_CUSTOM_PIN_PROFILE and every required FEEDMATE_PIN_* value.
#endif

#ifndef FEEDMATE_PIN_MOTOR_GATE
  #define FEEDMATE_PIN_MOTOR_GATE FEEDMATE_DEFAULT_MOTOR_GATE
#endif
#ifndef FEEDMATE_PIN_JAM_SENSOR
  #define FEEDMATE_PIN_JAM_SENSOR FEEDMATE_DEFAULT_JAM_SENSOR
#endif
#ifndef FEEDMATE_PIN_LIGHT_BARRIER
  #define FEEDMATE_PIN_LIGHT_BARRIER FEEDMATE_DEFAULT_LIGHT_BARRIER
#endif
#ifndef FEEDMATE_PIN_HALL_SENSOR
  #define FEEDMATE_PIN_HALL_SENSOR FEEDMATE_DEFAULT_HALL_SENSOR
#endif
#ifndef FEEDMATE_PIN_MANUAL_BUTTON
  #define FEEDMATE_PIN_MANUAL_BUTTON FEEDMATE_DEFAULT_MANUAL_BUTTON
#endif
#ifndef FEEDMATE_PIN_RTC_SDA
  #define FEEDMATE_PIN_RTC_SDA FEEDMATE_DEFAULT_RTC_SDA
#endif
#ifndef FEEDMATE_PIN_RTC_SCL
  #define FEEDMATE_PIN_RTC_SCL FEEDMATE_DEFAULT_RTC_SCL
#endif
#ifndef FEEDMATE_PIN_POWER_SENSE
  #define FEEDMATE_PIN_POWER_SENSE FEEDMATE_DEFAULT_POWER_SENSE
#endif
#ifndef FEEDMATE_PIN_STATUS_LED
  #define FEEDMATE_PIN_STATUS_LED FEEDMATE_DEFAULT_STATUS_LED
#endif

#define PIN_MOTOR_GATE    FEEDMATE_PIN_MOTOR_GATE
#define PIN_JAM_SENSOR    FEEDMATE_PIN_JAM_SENSOR
#define PIN_LIGHT_BARRIER FEEDMATE_PIN_LIGHT_BARRIER
#define PIN_HALL_SENSOR   FEEDMATE_PIN_HALL_SENSOR
#define PIN_MANUAL_BUTTON FEEDMATE_PIN_MANUAL_BUTTON
#define PIN_RTC_SDA       FEEDMATE_PIN_RTC_SDA
#define PIN_RTC_SCL       FEEDMATE_PIN_RTC_SCL
#define PIN_POWER_SENSE   FEEDMATE_PIN_POWER_SENSE
#define PIN_LED_BUILTIN   FEEDMATE_PIN_STATUS_LED
#define STATUS_LED_ON HIGH
#define STATUS_LED_OFF LOW

static_assert(PIN_MOTOR_GATE != PIN_JAM_SENSOR &&
              PIN_MOTOR_GATE != PIN_LIGHT_BARRIER &&
              PIN_MOTOR_GATE != PIN_HALL_SENSOR &&
              PIN_MOTOR_GATE != PIN_MANUAL_BUTTON &&
              PIN_MOTOR_GATE != PIN_RTC_SDA &&
              PIN_MOTOR_GATE != PIN_RTC_SCL &&
              PIN_MOTOR_GATE != PIN_POWER_SENSE,
              "The motor pin must be dedicated");
static_assert(PIN_RTC_SDA != PIN_RTC_SCL &&
              PIN_RTC_SDA != PIN_JAM_SENSOR && PIN_RTC_SCL != PIN_JAM_SENSOR &&
              PIN_RTC_SDA != PIN_HALL_SENSOR && PIN_RTC_SCL != PIN_HALL_SENSOR,
              "RTC pins must not share safety sensor inputs");
static_assert(PIN_JAM_SENSOR != PIN_HALL_SENSOR &&
              PIN_JAM_SENSOR != PIN_LIGHT_BARRIER &&
              PIN_JAM_SENSOR != PIN_MANUAL_BUTTON,
              "Jam input must be dedicated");

inline String deviceId() {
    uint8_t mac[6]{};
    // ESP.getEfuseMac() is stored in host integer order and produced a
    // byte-reversed SSID on little-endian ESP32 targets. Read the canonical
    // station MAC directly so labels match the address printed by esptool.
    if(esp_read_mac(mac,ESP_MAC_WIFI_STA)!=ESP_OK) return String("unknown");
    char id[13]; identity::formatMac(mac,id);
    return String(id);
}
// Password and fleet hashes from v1.1-v1.23 used the host-order eFuse
// representation as their device salt. Keep that salt stable so an upgrade
// does not invalidate credentials while the public device ID is corrected.
inline String credentialSaltId() {
    uint64_t mac=ESP.getEfuseMac();
    char id[13]; identity::formatLegacyEfuse(mac,id);
    return String(id);
}
inline void platformIdle() { delay(1); }
inline const char* platformName() { return FEEDMATE_PLATFORM_NAME; }
inline void setupStatusLed() {
    if(PIN_LED_BUILTIN >= 0) { pinMode(PIN_LED_BUILTIN,OUTPUT); digitalWrite(PIN_LED_BUILTIN,STATUS_LED_OFF); }
}
inline void writeStatusLed(bool active) {
    if(PIN_LED_BUILTIN >= 0) digitalWrite(PIN_LED_BUILTIN,active?STATUS_LED_ON:STATUS_LED_OFF);
}

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
inline String credentialSaltId() { return deviceId(); }
inline void platformIdle() { yield(); }
inline const char* platformName() { return "ESP8266"; }
inline void setupStatusLed() { pinMode(PIN_LED_BUILTIN,OUTPUT); digitalWrite(PIN_LED_BUILTIN,STATUS_LED_OFF); }
inline void writeStatusLed(bool active) { digitalWrite(PIN_LED_BUILTIN,active?STATUS_LED_ON:STATUS_LED_OFF); }
#else
#error Unsupported board: select a FeedMate PlatformIO environment.
#endif
