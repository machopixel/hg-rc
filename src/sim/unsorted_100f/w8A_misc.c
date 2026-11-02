#include "w8A.h"
#include "../util/access.h"
#include "../crt/hg_rest.h"

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
    /* C runtime replacement, the rest: standard C's time (crt/hg_rest.h) in place of Windows' clock and time zone
     * calls and the original's local-time-to-seconds 0x10105970 */
    int32_t t = hg_time();

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
