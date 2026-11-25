#pragma once
#include <Arduino.h>

// What the frame should show; filled by the controller / ceremony each loop.
struct Scene {
    uint8_t litMask    = 0;     // bit k-1 = candle #k lit (#1 = rightmost)
    bool    shamash    = false;
    int8_t  climbArm   = -1;    // candle number (1..8) whose wave is "climbing", or -1
    float   climb      = 0;     // 0..1 progress of the climbing light
    float   mastClimb  = -1;    // 0..1 while the mast fills up, -1 otherwise
    float   arrival    = -1;    // 0..1 while the "ship arrives" base sweep runs, -1 otherwise
};

enum Theme : uint8_t { THEME_SEA = 0, THEME_ISRAEL, THEME_GOLD, THEME_RAINBOW, THEME_COUNT };

namespace Lights {
    void begin();
    void setBrightness(uint8_t b);     // 0..255
    void render(const Scene &s, Theme theme, uint32_t nowMs);
}

// Candle outputs (flame LEDs through NPN transistors)
namespace Candles {
    void begin();
    void apply(uint8_t litMask, bool shamash);
}
