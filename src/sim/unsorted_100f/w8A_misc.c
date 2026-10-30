#include "w8A.h"
#include "../util/access.h"

typedef void(__stdcall *W8aGetTimeFn)(uint16_t *systemTime);
typedef uint32_t(__stdcall *W8aGetTimeZoneInformationFn)(uint8_t *info);

/* The original's frame: UTC SYSTEMTIME, local SYSTEMTIME, TIME_ZONE_INFORMATION (0xac bytes). */
typedef struct {
    uint16_t utc[8];
    uint16_t local[8];
    uint8_t tzi[0xac];
} W8aTimeFrame;

// FUNCTION: HGSIM 0x100ff500
int32_t __cdecl Crt_Time(int32_t *timer)
{
    W8aTimeFrame f;
    int32_t dst, t;

    SIM_API(W8aGetTimeFn, 0x10ac5650)(f.local);
    SIM_API(W8aGetTimeFn, 0x10ac561c)(f.utc);
    /* the cached daylight flag is kept while year, month, day, hour and minute (UTC) stay the same */
    if (f.utc[5] == *SIM_PTR(u16a, 0x1017e8ea) && f.utc[4] == *SIM_PTR(u16a, 0x1017e8e8) &&
        f.utc[3] == *SIM_PTR(u16a, 0x1017e8e6) && f.utc[1] == *SIM_PTR(u16a, 0x1017e8e2) &&
        f.utc[0] == *SIM_PTR(u16a, 0x1017e8e0)) {
        dst = GS32(0x1017e8d8);
    } else {
        uint32_t r = SIM_API(W8aGetTimeZoneInformationFn, 0x10ac564c)(f.tzi);

        if (r == 0xffffffffu)
            dst = -1;
        else if (r == 2 && U16(f.tzi, 0x9a) != 0 && U32(f.tzi, 0xa8) != 0)
            dst = 1;
        else
            dst = 0;
        G32(0x1017e8e0) = U32(f.utc, 0);
        G32(0x1017e8e4) = U32(f.utc, 4);
        G32(0x1017e8d8) = (uint32_t)dst;
        G32(0x1017e8e8) = U32(f.utc, 8);
        G32(0x1017e8ec) = U32(f.utc, 12);
    }
    t = W8A_FN_10105970(f.local[0], f.local[1], f.local[3], f.local[4], f.local[5], f.local[6], dst);
    if (timer != 0)
        *timer = t;
    return t;
}

// FUNCTION: HGSIM 0x100ffe20
void __cdecl Crt_NumToText(uint32_t val, char *buf, uint32_t radix, int32_t neg)
{
    char *p = buf;
    char *first;

    if (neg != 0) {
        *p++ = '-';
        val = 0u - val;
    }
    first = p;
    do {
        uint32_t digit = val % radix;

        val /= radix;
        *p++ = (char)(digit > 9 ? digit + 0x57 : digit + 0x30);
    } while (val != 0);
    *p-- = 0;
    do {
        char c = *first;

        *first = *p;
        *p = c;
        p--;
        first++;
    } while (first < p);
}
