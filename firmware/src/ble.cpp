#include "ble.h"
#include "app.h"
#include "config.h"
#include <NimBLEDevice.h>
#include <freertos/queue.h>

namespace {

struct Cmd { char text[200]; };

QueueHandle_t          queue    = nullptr;
NimBLEServer          *server   = nullptr;
NimBLECharacteristic  *chState  = nullptr;

volatile bool connected = false;
volatile bool dropAuth  = false;    // set on disconnect, handled in the main loop
bool     authed      = false;     // PIN accepted on the current connection
uint8_t  badPins     = 0;
uint32_t lockedUntil = 0;
uint32_t restartAt   = 0;         // pending restart after a WiFi change (0 = none)
String   lastJson;

class ServerCb : public NimBLEServerCallbacks {
    void onConnect(NimBLEServer *, ble_gap_conn_desc *) override { connected = true; }
    void onDisconnect(NimBLEServer *) override {
        connected = false;
        dropAuth  = true;                    // every connection must send the PIN
        // advertising restarts automatically (NimBLE default)
    }
};

class CmdCb : public NimBLECharacteristicCallbacks {
    void onWrite(NimBLECharacteristic *c) override {
        const std::string v = c->getValue();
        Cmd cmd;
        const size_t n = v.size() < sizeof cmd.text - 1 ? v.size() : sizeof cmd.text - 1;
        memcpy(cmd.text, v.data(), n);
        cmd.text[n] = 0;
        xQueueSend(queue, &cmd, 0);          // dropped if the queue is full
    }
};

String stateJson() {
    if (!authed) return "{\"auth\":false}";
    String s = App::stateJson();             // ends with '}'
    s.remove(s.length() - 1);
    s += ",\"auth\":true}";
    return s;
}

void execute(const char *text, uint32_t now) {
    String line(text);
    line.trim();
    const int eq = line.indexOf('=');
    const String key = eq < 0 ? line : line.substring(0, eq);
    const String val = eq < 0 ? String() : line.substring(eq + 1);

    if (key == "pin") {
        if (now < lockedUntil) return;
        if (val == OWNER_PIN) { authed = true; badPins = 0; }
        else if (++badPins >= 5) { badPins = 0; lockedUntil = now + 60000; }
        Serial.printf("BLE: PIN %s\n", authed ? "accepted" : "wrong");
        return;
    }
    if (!authed) return;

    if (key == "time") {
        const long epoch = val.toInt();
        if (epoch > 1700000000L) App::setClock((time_t)epoch);
    } else if (key == "mode") {
        if (val == "auto") App::setMode(Mode::Auto);
        else if (val == "manual") App::setMode(Mode::Manual);
        else if (val == "show") App::setMode(Mode::Show);
        else if (val == "demo") App::setMode(Mode::Demo);
    } else if (key == "night") {
        App::setManualNight((uint8_t)constrain(val.toInt(), 1, 8));
    } else if (key == "brightness") {
        App::setBrightness((uint8_t)constrain(val.toInt(), 0, 255));
    } else if (key == "theme") {
        App::setTheme((uint8_t)val.toInt());
    } else if (key == "light") {
        App::lightNow();
    } else if (key == "wifi") {
        const int tab = val.indexOf('\t');
        const String ssid = tab < 0 ? val : val.substring(0, tab);
        const String pass = tab < 0 ? String() : val.substring(tab + 1);
        App::saveWifi(ssid, pass);
        Serial.printf("BLE: home WiFi %s\n", ssid.length() ? ssid.c_str() : "(forgotten)");
        restartAt = now + 800;              // let the write response go out first
    }
}

} // namespace

void Ble::begin() {
    queue = xQueueCreate(8, sizeof(Cmd));
    NimBLEDevice::init(BLE_NAME);
    NimBLEDevice::setMTU(247);
    NimBLEDevice::setPower(ESP_PWR_LVL_P9);

    server = NimBLEDevice::createServer();
    server->setCallbacks(new ServerCb());

    NimBLEService *svc = server->createService(BLE_SVC_UUID);
    chState = svc->createCharacteristic(BLE_STATE_UUID, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY, 512);
    NimBLECharacteristic *chCmd = svc->createCharacteristic(BLE_CMD_UUID, NIMBLE_PROPERTY::WRITE, 512);
    chCmd->setCallbacks(new CmdCb());
    lastJson = stateJson();
    chState->setValue(lastJson.c_str());
    svc->start();

    NimBLEAdvertising *adv = NimBLEDevice::getAdvertising();
    adv->addServiceUUID(BLE_SVC_UUID);
    adv->setScanResponse(true);              // the name goes into the scan response
    adv->start();
    Serial.println("BLE advertising as \"" BLE_NAME "\"");
}

void Ble::handle() {
    const uint32_t now = millis();

    if (dropAuth) { dropAuth = false; authed = false; }

    Cmd cmd;
    while (xQueueReceive(queue, &cmd, 0) == pdTRUE) execute(cmd.text, now);

    if (restartAt && (int32_t)(now - restartAt) >= 0) ESP.restart();

    // publish the state when it changes (checked at most every 250 ms)
    static uint32_t lastCheck = 0;
    if (now - lastCheck < 250) return;
    lastCheck = now;
    const String js = stateJson();
    if (js != lastJson) {
        lastJson = js;
        chState->setValue(js.c_str());
        if (connected) chState->notify();
    }
}
