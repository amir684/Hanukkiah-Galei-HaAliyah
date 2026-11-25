#include "lights.h"
#include "config.h"
#include <FastLED.h>

// ---------------------------------------------------------------- layout
static constexpr uint16_t armTotal(uint8_t i = 0) {
    return i >= 8 ? 0 : ARM_LEDS[i] + armTotal(i + 1);
}
static constexpr uint16_t ARMS_N = armTotal();
static constexpr uint16_t COB_N  = BASE_LEDS + MAST_LEDS;

static CRGB arms[ARMS_N];
static CRGB cob[COB_N];
static uint16_t armStart[8];

// chain index 0..7 = L4 L3 L2 L1 R1 R2 R3 R4 ; candle #1 = R4 (index 7)
static inline uint8_t candleOfArm(uint8_t a) { return 8 - a; }

// ---------------------------------------------------------------- themes
struct Palette { CRGB deep, light, foam, litA, litB; };
static const Palette PALETTES[THEME_COUNT] = {
    // sea: blue water, golden lit waves
    {CRGB(0, 18, 70),  CRGB(0, 120, 200), CRGB(170, 220, 255), CRGB(255, 120, 10), CRGB(255, 200, 90)},
    // Israel: blue & white
    {CRGB(0, 30, 140), CRGB(20, 80, 255), CRGB(255, 255, 255), CRGB(120, 160, 255), CRGB(255, 255, 255)},
    // gold: warm amber everything
    {CRGB(60, 18, 0),  CRGB(160, 70, 5),  CRGB(255, 190, 90),  CRGB(255, 110, 0),  CRGB(255, 210, 120)},
    // rainbow: handled procedurally (deep/light unused)
    {CRGB::Black, CRGB::Black, CRGB(255, 255, 255), CRGB(255, 140, 20), CRGB(255, 220, 140)},
};

static inline CRGB mix(const CRGB &a, const CRGB &b, float t) {
    t = constrain(t, 0.f, 1.f);
    return CRGB(a.r + (b.r - a.r) * t, a.g + (b.g - a.g) * t, a.b + (b.b - a.b) * t);
}
static inline CRGB scale(CRGB c, float k) {
    k = constrain(k, 0.f, 1.f);
    return CRGB(c.r * k, c.g * k, c.b * k);
}
static inline float wave01(float x) { return 0.5f + 0.5f * sinf(x * TWO_PI); }

// ---------------------------------------------------------------- API
void Lights::begin() {
    uint16_t s = 0;
    for (uint8_t i = 0; i < 8; i++) { armStart[i] = s; s += ARM_LEDS[i]; }
    FastLED.addLeds<WS2812B, PIN_ARMS_DATA, ARMS_COLOR_ORDER>(arms, ARMS_N);
    FastLED.addLeds<WS2812B, PIN_COB_DATA, COB_COLOR_ORDER>(cob, COB_N);
    FastLED.setMaxPowerInVoltsAndMilliamps(5, MAX_MILLIAMPS);
    FastLED.setDither(0);
    FastLED.clear(true);
}

void Lights::setBrightness(uint8_t b) { FastLED.setBrightness(b); }

void Lights::render(const Scene &s, Theme theme, uint32_t nowMs) {
    const float t = nowMs / 1000.0f;
    const Palette &p = PALETTES[theme];

    // ---- the 8 wave arms ----
    for (uint8_t a = 0; a < 8; a++) {
        const uint8_t candle = candleOfArm(a);
        const bool lit = s.litMask & (1 << (candle - 1));
        const uint8_t n = ARM_LEDS[a];
        for (uint8_t i = 0; i < n; i++) {
            const float pos = n > 1 ? (float)i / (n - 1) : 0;          // 0 bottom .. 1 crest
            const float w = wave01(pos * 1.2f - t * 0.25f + a * 0.11f); // swell rolling upward
            CRGB c;
            if (theme == THEME_RAINBOW) {
                c = CHSV((uint8_t)(a * 24 + pos * 70 + t * 18), 230, 255);
                c = scale(c, lit ? 0.55f + 0.45f * w : 0.12f + 0.25f * w);
            } else if (lit) {
                c = scale(mix(p.litA, p.litB, w), 0.6f + 0.4f * w);
            } else {
                c = scale(mix(p.deep, p.light, w), 0.3f + 0.5f * w);
            }
            if (pos > 0.82f) {                                          // foam near the crest
                const float f = (pos - 0.82f) / 0.18f * (lit ? 0.6f : 0.35f) * w;
                c = mix(c, p.foam, f);
            }
            // light climbing up this wave toward its candle
            if (s.climbArm == candle) {
                const float d = s.climb - pos;
                if (d >= 0) c = mix(c, p.foam, d < 0.08f ? 1.0f : 0.55f);  // head + trail
                else c = scale(c, 0.25f);
            }
            arms[armStart[a] + i] = c;
        }
    }

    // ---- base window: horizontal sea ----
    for (uint16_t i = 0; i < BASE_LEDS; i++) {
        const float x = BASE_LEDS > 1 ? (float)i / (BASE_LEDS - 1) : 0;
        const float w = wave01(x * 2.0f - t * 0.3f) * 0.6f + wave01(x * 3.3f + t * 0.17f) * 0.4f;
        CRGB c = (theme == THEME_RAINBOW) ? CRGB(CHSV((uint8_t)(x * 120 + t * 25), 220, 255))
                                          : mix(p.deep, p.light, w);
        c = scale(c, 0.45f + 0.55f * w);
        if (w > 0.85f) c = mix(c, p.foam, (w - 0.85f) / 0.15f);
        if (s.arrival >= 0) {                                       // bright band sweeping in
            const float band = fabsf(fabsf(x - 0.5f) * 2.0f - (1.0f - s.arrival));
            if (band < 0.18f) c = mix(c, p.foam, 1.0f - band / 0.18f);
        }
        cob[i] = c;
    }

    // ---- mast (shamash column) ----
    const CRGB warm(255, 170, 70);
    for (uint16_t i = 0; i < MAST_LEDS; i++) {
        const float pos = MAST_LEDS > 1 ? (float)i / (MAST_LEDS - 1) : 0;
        CRGB c;
        if (s.mastClimb >= 0) {
            c = pos <= s.mastClimb ? mix(warm, CRGB::White, pos > s.mastClimb - 0.06f ? 1.f : 0.f)
                                   : scale(p.deep, 0.4f);
        } else if (s.shamash) {
            c = scale(warm, 0.65f + 0.35f * wave01(pos * 0.8f - t * 0.2f));
        } else {
            c = scale(mix(p.deep, p.light, wave01(pos - t * 0.2f)), 0.5f);
        }
        cob[BASE_LEDS + i] = c;
    }

    FastLED.show();
}

// ---------------------------------------------------------------- candles
void Candles::begin() {
    for (uint8_t i = 0; i < 8; i++) { pinMode(PIN_CANDLE[i], OUTPUT); digitalWrite(PIN_CANDLE[i], LOW); }
    pinMode(PIN_SHAMASH, OUTPUT);
    digitalWrite(PIN_SHAMASH, LOW);
}

void Candles::apply(uint8_t litMask, bool shamash) {
    for (uint8_t i = 0; i < 8; i++) digitalWrite(PIN_CANDLE[i], (litMask >> i) & 1);
    digitalWrite(PIN_SHAMASH, shamash);
}
