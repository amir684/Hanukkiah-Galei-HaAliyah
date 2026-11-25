#include "web.h"
#include "app.h"
#include "web_page.h"
#include <WebServer.h>

static WebServer server(80);

static void sendState() { server.send(200, "application/json", App::stateJson()); }

static void handleSet() {
    if (server.hasArg("mode")) {
        const String m = server.arg("mode");
        if (m == "auto") App::setMode(Mode::Auto);
        else if (m == "manual") App::setMode(Mode::Manual);
        else if (m == "show") App::setMode(Mode::Show);
        else if (m == "demo") App::setMode(Mode::Demo);
    }
    if (server.hasArg("night"))      App::setManualNight((uint8_t)server.arg("night").toInt());
    if (server.hasArg("brightness")) App::setBrightness((uint8_t)constrain(server.arg("brightness").toInt(), 0, 255));
    if (server.hasArg("theme"))      App::setTheme((uint8_t)server.arg("theme").toInt());
    sendState();
}

static void handleTime() {
    const long epoch = server.arg("epoch").toInt();
    if (epoch > 1700000000L) App::setClock((time_t)epoch);
    sendState();
}

static void handleWifi() {
    const String ssid = server.arg("ssid");
    if (!ssid.length()) { server.send(400, "text/plain", "missing ssid"); return; }
    App::saveWifi(ssid, server.arg("pass"));
    server.send(200, "text/plain; charset=utf-8", "OK - restarting");
    delay(500);
    ESP.restart();
}

void Web::begin() {
    server.on("/", HTTP_GET, [] { server.send_P(200, "text/html; charset=utf-8", PAGE_HTML); });
    server.on("/api/state", HTTP_GET, sendState);
    server.on("/api/set", HTTP_GET, handleSet);
    server.on("/api/light", HTTP_GET, [] { App::lightNow(); sendState(); });
    server.on("/api/time", HTTP_GET, handleTime);
    server.on("/api/wifi", HTTP_POST, handleWifi);
    server.onNotFound([] { server.sendHeader("Location", "/"); server.send(302); });
    server.begin();
}

void Web::handle() { server.handleClient(); }
