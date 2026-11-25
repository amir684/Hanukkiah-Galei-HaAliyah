#pragma once

// Bluetooth (BLE) control, used by the web page docs/index.html (Web Bluetooth).
//
// One GATT service (UUIDs in config.h) with two characteristics:
//   state   read + notify   JSON. {"auth":false} until the PIN was sent on this
//                           connection, then the full state (same as /api/state)
//                           plus "auth":true. A notify means "changed, read again"
//                           (its payload may be cut to the MTU).
//   command write           UTF-8 text "key=value":
//     pin=<PIN>                      unlock this connection
//     time=<unix epoch>              set the clock from the phone
//     mode=auto|manual|show|demo
//     night=1..8
//     brightness=0..255
//     theme=0..3
//     light                          run the lighting ceremony
//     wifi=<ssid>\t<password>        save the home network, then restart
//     wifi=                          forget the home network, then restart
// Everything except "pin" needs the PIN first. 5 wrong PINs lock it for 60 s.
//
// Callbacks run on the BLE task; commands are queued and executed in handle()
// on the main loop, so all state changes stay on one task.
namespace Ble {
    void begin();
    void handle();      // call from loop()
}
