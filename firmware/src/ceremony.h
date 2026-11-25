#pragma once
#include "lights.h"

// The lighting animation:
//   (optional) the ship arrives - a bright sweep on the base window
//   -> light climbs the mast and lights the shamash
//   -> for each candle, from the newest (leftmost) to the right:
//      light climbs its wave, then the candle catches fire.
class Ceremony {
public:
    void start(uint8_t night, bool withArrival, uint32_t now);
    bool active() const { return running; }
    // Fills `s` while running; returns false once finished.
    bool update(uint32_t now, Scene &s);

private:
    static constexpr uint32_t ARRIVAL_MS = 1600;
    static constexpr uint32_t MAST_MS    = 1300;
    static constexpr uint32_t CLIMB_MS   = 850;
    static constexpr uint32_t GAP_MS     = 300;

    bool running = false;
    uint8_t night = 0;
    bool arrival = false;
    uint32_t t0 = 0;
};
