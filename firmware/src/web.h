#pragma once

// Phone control page + small JSON API
//   GET  /                 control page (Hebrew)
//   GET  /api/state        current state (JSON)
//   GET  /api/set?mode=auto|manual|show|demo&night=1..8&brightness=0..255&theme=0..3
//   GET  /api/light        run the lighting ceremony
//   GET  /api/time?epoch=N set the clock (the page does this from the phone)
//   POST /api/wifi         ssid, pass -> saved, then restart
namespace Web {
    void begin();
    void handle();
}
