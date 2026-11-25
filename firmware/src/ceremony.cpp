#include "ceremony.h"

void Ceremony::start(uint8_t n, bool withArrival, uint32_t now) {
    night = n > 8 ? 8 : n;
    arrival = withArrival;
    t0 = now;
    running = true;
}

bool Ceremony::update(uint32_t now, Scene &s) {
    if (!running) return false;
    uint32_t e = now - t0;
    s = Scene();

    if (arrival) {
        if (e < ARRIVAL_MS) { s.arrival = (float)e / ARRIVAL_MS; return true; }
        e -= ARRIVAL_MS;
    }
    if (e < MAST_MS) { s.mastClimb = (float)e / MAST_MS; return true; }
    e -= MAST_MS;
    s.shamash = true;

    // Candles are placed from the right (#1) but lit from the newest one:
    // night n lights #n first, then #n-1 ... #1.
    for (uint8_t j = 0; j < night; j++) {
        const uint8_t candle = night - j;
        if (e < CLIMB_MS) {
            s.climbArm = candle;
            s.climb = (float)e / CLIMB_MS;
            return true;
        }
        e -= CLIMB_MS;
        s.litMask |= 1 << (candle - 1);
        if (e < GAP_MS) return true;
        e -= GAP_MS;
    }
    running = false;
    return false;
}
