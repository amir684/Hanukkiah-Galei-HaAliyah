// =====================================================================
//  Hanukkiah "Galei HaAliyah" - main controller
//  ESP32-S3 SuperMini
// =====================================================================
#include <Arduino.h>
#include <Preferences.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <sys/time.h>

#include "config.h"
#include "app.h"
#include "lights.h"
#include "ceremony.h"
#include "hebcal.h"
#include "web.h"
#include "ble.h"

static Settings   cfg;
static Preferences prefs;
static Ceremony   ceremony;

static uint8_t  shownNight = 0;      // night currently displayed (0 = nothing yet)
static uint8_t  demoNight  = 0;
static uint32_t idleSince  = 0;      // when the last ceremony finished
static bool     wasRunning = false;

// ---------------------------------------------------------------- settings
static void loadSettings() {
    prefs.begin("hk", true);
    cfg.mode        = (Mode)prefs.getUChar("mode", (uint8_t)Mode::Auto);
    cfg.manualNight = prefs.getUChar("night", 1);
    cfg.brightness  = prefs.getUChar("bright", 150);
    cfg.theme       = prefs.getUChar("theme", 0);
    prefs.end();
    if ((uint8_t)cfg.mode > (uint8_t)Mode::Demo) cfg.mode = Mode::Auto;
    cfg.manualNight = constrain(cfg.manualNight, 1, 8);
    if (cfg.theme >= THEME_COUNT) cfg.theme = 0;
}

static void saveSettings() {
    prefs.begin("hk", false);
    prefs.putUChar("mode", (uint8_t)cfg.mode);
    prefs.putUChar("night", cfg.manualNight);
    prefs.putUChar("bright", cfg.brightness);
    prefs.putUChar("theme", cfg.theme);
    prefs.end();
}

// ---------------------------------------------------------------- time
static bool clockValid() { return time(nullptr) > 1700000000; }

static bool localNow(struct tm &t) {
    if (!clockValid()) return false;
    time_t now = time(nullptr);
    localtime_r(&now, &t);
    return true;
}

// ---------------------------------------------------------------- App API (used by web.cpp)
Settings &App::settings() { return cfg; }

const char *App::modeName(Mode m) {
    switch (m) {
        case Mode::Auto:   return "auto";
        case Mode::Manual: return "manual";
        case Mode::Show:   return "show";
        default:           return "demo";
    }
}

void App::setMode(Mode m)            { cfg.mode = m; saveSettings(); }
void App::setManualNight(uint8_t n)  { cfg.manualNight = constrain(n, 1, 8); cfg.mode = Mode::Manual; saveSettings(); }
void App::setBrightness(uint8_t b)   { cfg.brightness = b; Lights::setBrightness(b); saveSettings(); }
void App::setTheme(uint8_t t)        { cfg.theme = t < THEME_COUNT ? t : 0; saveSettings(); }
void App::lightNow()                 { ceremony.start(shownNight ? shownNight : 8, true, millis()); }

void App::setClock(time_t epoch) {
    struct timeval tv = {epoch, 0};
    settimeofday(&tv, nullptr);
    Serial.printf("Clock set from phone: %ld\n", (long)epoch);
}

void App::saveWifi(const String &ssid, const String &pass) {
    prefs.begin("hk", false);
    prefs.putString("ssid", ssid);
    prefs.putString("pass", pass);
    prefs.end();
}

String App::stateJson() {
    struct tm t;
    const bool valid = localNow(t);
    char timeStr[24] = "", eveStr[16] = "";
    uint8_t hNight = 0;
    if (valid) {
        strftime(timeStr, sizeof timeStr, "%Y-%m-%d %H:%M", &t);
        hNight = hanukkahNight(t, LIGHT_HOUR, LIGHT_MINUTE);
        int32_t y; unsigned m, d;
        daysToCivil(nextHanukkahEve(t), y, m, d);
        snprintf(eveStr, sizeof eveStr, "%04ld-%02u-%02u", (long)y, m, d);
    }
    const bool sta = WiFi.status() == WL_CONNECTED;
    char buf[400];
    snprintf(buf, sizeof buf,
        "{\"mode\":\"%s\",\"night\":%u,\"manualNight\":%u,\"brightness\":%u,\"theme\":%u,"
        "\"timeValid\":%s,\"time\":\"%s\",\"hanukkahNight\":%u,\"nextEve\":\"%s\","
        "\"sta\":%s,\"ip\":\"%s\",\"ssid\":\"%s\",\"busy\":%s}",
        modeName(cfg.mode), shownNight, cfg.manualNight, cfg.brightness, cfg.theme,
        valid ? "true" : "false", timeStr, hNight, eveStr,
        sta ? "true" : "false", sta ? WiFi.localIP().toString().c_str() : "",
        sta ? WiFi.SSID().c_str() : "", ceremony.active() ? "true" : "false");
    return String(buf);
}

// ---------------------------------------------------------------- inputs
static void handleButton(uint32_t now) {
    static bool down = false, longDone = false;
    static uint32_t pressedAt = 0, lastChange = 0;
    const bool pressed = digitalRead(PIN_BUTTON) == LOW;
    if (pressed != down && now - lastChange > 30) {             // debounce
        lastChange = now;
        down = pressed;
        if (pressed) { pressedAt = now; longDone = false; }
        else if (!longDone) {                                    // short press: next night
            if (cfg.mode == Mode::Manual) {
                if (cfg.manualNight < 8) App::setManualNight(cfg.manualNight + 1);
                else App::setMode(Mode::Show);
            } else if (cfg.mode == Mode::Show) {
                App::setManualNight(1);
            } else {
                App::setManualNight(shownNight % 8 + 1);
            }
        }
    }
    if (down && !longDone && now - pressedAt > LONG_PRESS_MS) {  // long press: automatic
        longDone = true;
        App::setMode(Mode::Auto);
        shownNight = 0;                  // force a fresh lighting ceremony as feedback
    }
}

static void handleTouch(uint32_t now) {
    static bool last = false;
    static uint32_t lastFire = 0;
    const bool on = digitalRead(PIN_TOUCH) == HIGH;
    if (on && !last && now - lastFire > 400) {
        lastFire = now;
        App::lightNow();
    }
    last = on;
}

// ---------------------------------------------------------------- which night to show
static uint8_t demoStep(uint32_t now) {
    if (demoNight == 0) demoNight = 1;
    if (!ceremony.active() && now - idleSince > DEMO_HOLD_MS) {
        demoNight = demoNight % 8 + 1;
    }
    return demoNight;
}

static uint8_t targetNight(uint32_t now) {
    switch (cfg.mode) {
        case Mode::Manual: return cfg.manualNight;
        case Mode::Show:   return 8;
        case Mode::Demo:   return demoStep(now);
        case Mode::Auto:
        default: {
            struct tm t;
            if (localNow(t)) {
                const uint8_t n = hanukkahNight(t, LIGHT_HOUR, LIGHT_MINUTE);
                if (n) return n;
            }
            return demoStep(now);   // not Hanukkah (or no clock): exhibition demo
        }
    }
}

// ---------------------------------------------------------------- WiFi
static void startWifi() {
    WiFi.mode(WIFI_AP_STA);
    WiFi.softAP(AP_SSID, AP_PASS);
    prefs.begin("hk", true);
    const String ssid = prefs.getString("ssid", "");
    const String pass = prefs.getString("pass", "");
    prefs.end();
    if (ssid.length()) {
        WiFi.setAutoReconnect(true);
        WiFi.begin(ssid.c_str(), pass.c_str());
        Serial.printf("Connecting to %s ...\n", ssid.c_str());
    }
    configTzTime(TZ_ISRAEL, "pool.ntp.org", "time.google.com");
    MDNS.begin(MDNS_NAME);
    MDNS.addService("http", "tcp", 80);
    Serial.printf("AP \"%s\" -> http://%s\n", AP_SSID, WiFi.softAPIP().toString().c_str());
}

// ---------------------------------------------------------------- setup / loop
void setup() {
    Serial.begin(115200);
    pinMode(PIN_BUTTON, INPUT_PULLUP);
    pinMode(PIN_TOUCH, INPUT);
    Candles::begin();
    Lights::begin();
    loadSettings();
    Lights::setBrightness(cfg.brightness);
    startWifi();
    Web::begin();
    Ble::begin();
    idleSince = millis();
}

void loop() {
    const uint32_t now = millis();
    Web::handle();
    Ble::handle();
    handleButton(now);
    handleTouch(now);

    const uint8_t target = targetNight(now);
    if (target != shownNight && !(ceremony.active() && cfg.mode == Mode::Demo)) {
        shownNight = target;
        ceremony.start(target, false, now);
    }

    Scene scene;
    if (!ceremony.update(now, scene)) {
        scene.litMask = (1 << shownNight) - 1;
        scene.shamash = shownNight > 0;
    }
    if (wasRunning && !ceremony.active()) idleSince = now;
    wasRunning = ceremony.active();

    Candles::apply(scene.litMask, scene.shamash);

    static uint32_t lastFrame = 0;
    if (now - lastFrame >= 16) {               // ~60 fps
        lastFrame = now;
        Lights::render(scene, (Theme)cfg.theme, now);
    }
}
