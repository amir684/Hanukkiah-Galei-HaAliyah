#pragma once
#include <stdint.h>
#include <time.h>

// Hebrew-calendar helpers for Hanukkah (Dershowitz & Reingold arithmetic).
// Verified against the actual first-candle evenings for 2021-2035.

// Day number (days since 1970-01-01) of a civil date.
int32_t civilToDays(int32_t y, unsigned m, unsigned d);
void daysToCivil(int32_t z, int32_t &y, unsigned &m, unsigned &d);

// Day number of the evening of the FIRST candle (= 24 Kislev) for the
// Hanukkah that falls in Nov/Dec of the given Gregorian year.
int32_t hanukkahFirstEve(int32_t gregorianYear);

// Which Hanukkah night is it at local time `t` (1..8), or 0 if not Hanukkah.
// A night starts at lightHour:lightMinute and lasts until the next evening.
uint8_t hanukkahNight(const struct tm &t, uint8_t lightHour, uint8_t lightMinute);

// First-candle evening of the current Hanukkah, or the next one (day number).
int32_t nextHanukkahEve(const struct tm &t);
