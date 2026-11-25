#pragma once
// =====================================================================
//  Hardware configuration - edit here if your wiring or LED counts differ
// =====================================================================
#include <Arduino.h>

// ---------- pins (ESP32-S3 SuperMini) ----------
// Candle #1 is the RIGHTMOST candle as seen from the front (the first night's
// candle); #8 is the leftmost. Each pin drives an NPN transistor (low side).
constexpr uint8_t PIN_CANDLE[8] = {1, 2, 4, 5, 6, 7, 8, 9};
constexpr uint8_t PIN_SHAMASH   = 10;

constexpr uint8_t PIN_ARMS_DATA = 11;   // WS2812B 2020 (2.7 mm) - the 8 wave arms
constexpr uint8_t PIN_COB_DATA  = 12;   // WS2812B COB (5 mm)    - base window, then mast
constexpr uint8_t PIN_TOUCH     = 13;   // TTP223 output (HIGH while touched)
constexpr uint8_t PIN_BUTTON    = 44;   // push button to GND (pin labelled RX)

// ---------- LED counts ----------
// Arms in DATA-CHAIN order, as seen from the front: L4 L3 L2 L1 | R1 R2 R3 R4
// (outer-left first). Each arm: data enters at the bottom, leaves at the top,
// and a thin wire brings it back down to the next arm's bottom.
// Count the LEDs you actually cut for each arm and put the numbers here.
// Counted by the user on the real strips (2026-11-25).
constexpr uint8_t  ARM_LEDS[8] = {24, 22, 22, 22, 22, 22, 22, 24};
constexpr uint16_t BASE_LEDS   = 22;    // COB strip behind the base window (left -> right)
constexpr uint16_t MAST_LEDS   = 34;    // COB strip in the mast (bottom -> top)

// Colour order: most WS2812B are GRB. If red and green look swapped, change it.
#define ARMS_COLOR_ORDER GRB
#define COB_COLOR_ORDER  GRB

// Current budget for all strips (mA). With a 5V/3A charger: 2200 for the strips
// + ~250 ESP32 + ~270 for the 9 flame LEDs stays under 3 A.
constexpr uint32_t MAX_MILLIAMPS = 2200;

// ---------- WiFi ----------
// The hanukkiah always opens its own network (works anywhere, no internet needed).
#define AP_SSID   "Hanukkiah"
#define MDNS_NAME "hanukkiah"       // http://hanukkiah.local when on your home WiFi

// AP_PASS and OWNER_PIN live in secrets.h, which is not uploaded to GitHub.
#if __has_include("secrets.h")
#include "secrets.h"
#else
#warning "include/secrets.h is missing - using the example values. Copy secrets.example.h to secrets.h."
#include "secrets.example.h"
#endif

// ---------- Bluetooth (BLE) ----------
// The control page (docs/index.html) connects with Web Bluetooth.
// These UUIDs must match the page.
#define BLE_NAME      "Hanukkiah"
#define BLE_SVC_UUID  "8c2b1e00-7d3a-4f6e-9b1c-5a4d3e2f1a00"
#define BLE_STATE_UUID "8c2b1e01-7d3a-4f6e-9b1c-5a4d3e2f1a00"   // read + notify (JSON)
#define BLE_CMD_UUID   "8c2b1e02-7d3a-4f6e-9b1c-5a4d3e2f1a00"   // write ("key=value")

// ---------- time ----------
// Israel time zone incl. daylight saving
#define TZ_ISRAEL "IST-2IDT,M3.4.4/26,M10.5.0"
// When the "night" changes to the next one (the evening lighting time)
constexpr uint8_t LIGHT_HOUR   = 16;
constexpr uint8_t LIGHT_MINUTE = 30;

// ---------- timing ----------
constexpr uint32_t DEMO_HOLD_MS  = 9000;  // demo: how long each night stays lit
constexpr uint32_t LONG_PRESS_MS = 1200;  // button long press -> automatic mode
