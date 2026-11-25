#pragma once
#include <Arduino.h>
#include <time.h>

enum class Mode : uint8_t {
    Auto = 0,   // follows the Hebrew calendar; outside Hanukkah behaves like Demo
    Manual,     // a fixed night chosen with the button / phone
    Show,       // all 8 candles
    Demo        // cycles nights 1..8 with the lighting animation (exhibition)
};

struct Settings {
    Mode    mode        = Mode::Auto;
    uint8_t manualNight = 1;
    uint8_t brightness  = 150;
    uint8_t theme       = 0;
};

// Shared between main.cpp and web.cpp
namespace App {
    Settings &settings();
    void setMode(Mode m);
    void setManualNight(uint8_t n);
    void setBrightness(uint8_t b);
    void setTheme(uint8_t t);
    void lightNow();                       // run the full lighting ceremony
    void setClock(time_t epoch);           // from the phone, when there is no internet
    void saveWifi(const String &ssid, const String &pass);
    String stateJson();
    const char *modeName(Mode m);
}
