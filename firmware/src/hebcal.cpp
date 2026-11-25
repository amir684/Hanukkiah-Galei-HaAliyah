#include "hebcal.h"

// --- civil date <-> day number (Howard Hinnant's algorithms) ---
int32_t civilToDays(int32_t y, unsigned m, unsigned d) {
    y -= m <= 2;
    const int32_t era = (y >= 0 ? y : y - 399) / 400;
    const unsigned yoe = (unsigned)(y - era * 400);
    const unsigned doy = (153 * (m + (m > 2 ? -3 : 9)) + 2) / 5 + d - 1;
    const unsigned doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
    return era * 146097 + (int32_t)doe - 719468;
}

void daysToCivil(int32_t z, int32_t &y, unsigned &m, unsigned &d) {
    z += 719468;
    const int32_t era = (z >= 0 ? z : z - 146096) / 146097;
    const unsigned doe = (unsigned)(z - era * 146097);
    const unsigned yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365;
    y = (int32_t)yoe + era * 400;
    const unsigned doy = doe - (365 * yoe + yoe / 4 - yoe / 100);
    const unsigned mp = (5 * doy + 2) / 153;
    d = doy - (153 * mp + 2) / 5 + 1;
    m = mp < 10 ? mp + 3 : mp - 9;
    y += (m <= 2);
}

// --- Hebrew calendar ---
static int32_t elapsedDays(int32_t y) {
    const int32_t months = (235 * y - 234) / 19;
    const int64_t parts = 12084LL + 13753LL * months;
    int32_t day = months * 29 + (int32_t)(parts / 25920);
    if ((3 * (day + 1)) % 7 < 3) day++;
    return day;
}

static int32_t newYearDelay(int32_t y) {
    const int32_t a = elapsedDays(y - 1), b = elapsedDays(y), c = elapsedDays(y + 1);
    if (c - b == 356) return 2;
    if (b - a == 382) return 1;
    return 0;
}

static const int32_t HEBREW_EPOCH_RD = -1373427;   // fixed (R.D.) day of 1 Tishri AM 1
static const int32_t RD_UNIX_EPOCH  = 719163;      // R.D. of 1970-01-01

static int32_t newYearRD(int32_t y) { return HEBREW_EPOCH_RD + elapsedDays(y) + newYearDelay(y); }

int32_t hanukkahFirstEve(int32_t gregorianYear) {
    const int32_t hy = gregorianYear + 3761;           // Hebrew year starting that autumn
    const int32_t ny = newYearRD(hy);
    const int32_t len = newYearRD(hy + 1) - ny;
    const int32_t heshvan = (len % 10 == 5) ? 30 : 29;  // long year -> 30-day Heshvan
    const int32_t kislev25 = ny + 30 + heshvan + 24;   // Tishri(30) + Heshvan + 24 days
    return kislev25 - 1 - RD_UNIX_EPOCH;               // first candle: evening before
}

static int32_t eveningDay(const struct tm &t, uint8_t lh, uint8_t lm) {
    const int32_t today = civilToDays(t.tm_year + 1900, t.tm_mon + 1, t.tm_mday);
    const bool afterLighting = (t.tm_hour > lh) || (t.tm_hour == lh && t.tm_min >= lm);
    return afterLighting ? today : today - 1;
}

uint8_t hanukkahNight(const struct tm &t, uint8_t lightHour, uint8_t lightMinute) {
    const int32_t e = eveningDay(t, lightHour, lightMinute);
    const int32_t year = t.tm_year + 1900;
    for (int32_t g = year; g >= year - 1; g--) {        // Hanukkah can run into January
        const int32_t n = e - hanukkahFirstEve(g) + 1;
        if (n >= 1 && n <= 8) return (uint8_t)n;
    }
    return 0;
}

int32_t nextHanukkahEve(const struct tm &t) {
    const int32_t today = civilToDays(t.tm_year + 1900, t.tm_mon + 1, t.tm_mday);
    const int32_t year = t.tm_year + 1900;
    const int32_t prev = hanukkahFirstEve(year - 1);
    if (today <= prev + 8) return prev;                 // still in last year's Hanukkah (January)
    const int32_t cur = hanukkahFirstEve(year);
    if (today <= cur + 8) return cur;
    return hanukkahFirstEve(year + 1);
}
