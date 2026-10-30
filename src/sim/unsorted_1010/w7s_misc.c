#include "w7s.h"
#include "w6k.h"
#include "../util/access.h"

// FUNCTION: HGSIM 0x101086c0
uint32_t __cdecl Sim_101086c0(uint32_t newv, uint32_t mask)
{
    return W7S_FN_10108680(newv, mask & 0xfff7ffffu);
}

/* The 16-bit field at addr, zero-extended. */
#define W16(addr) ((int32_t)*SIM_PTR(const uint16_t, addr))

// FUNCTION: HGSIM 0x1010a0a0
int32_t __cdecl Sim_1010a0a0(const uint8_t *tm)
{
    int32_t year;
    int32_t start;
    int32_t end;
    int32_t yday;
    int32_t ms;

    if (G32(0x1015c024) == 0)
        return 0;
    year = S32(tm, 0x14);
    if (year != GS32(0x1015c0b8) || year != GS32(0x1015c0c8)) {
        if (G32(0x1017ec28) != 0) {
            /* two SYSTEMTIMEs: 0x1017ecc8 (start) and 0x1017ec74 (end): year, month, weekday, day, hour,
             * minute, second, milliseconds */
            if (W16(0x1017ecc8) == 0)
                Sim_1010a310(1, 1, year, W16(0x1017ecca), W16(0x1017ecce), W16(0x1017eccc), 0, W16(0x1017ecd0),
                             W16(0x1017ecd2), W16(0x1017ecd4), W16(0x1017ecd6));
            else
                Sim_1010a310(1, 0, year, W16(0x1017ecca), 0, 0, W16(0x1017ecce), W16(0x1017ecd0), W16(0x1017ecd2),
                             W16(0x1017ecd4), W16(0x1017ecd6));
            if (W16(0x1017ec74) == 0)
                Sim_1010a310(0, 1, S32(tm, 0x14), W16(0x1017ec76), W16(0x1017ec7a), W16(0x1017ec78), 0,
                             W16(0x1017ec7c), W16(0x1017ec7e), W16(0x1017ec80), W16(0x1017ec82));
            else
                Sim_1010a310(0, 0, S32(tm, 0x14), W16(0x1017ec76), 0, 0, W16(0x1017ec7a), W16(0x1017ec7c),
                             W16(0x1017ec7e), W16(0x1017ec80), W16(0x1017ec82));
        } else {
            Sim_1010a310(1, 1, year, 4, 1, 0, 0, 2, 0, 0, 0);
            Sim_1010a310(0, 1, S32(tm, 0x14), 10, 5, 0, 0, 2, 0, 0, 0);
        }
    }
    start = GS32(0x1015c0bc);
    end = GS32(0x1015c0cc);
    yday = S32(tm, 0x1c);
    if (start < end) {
        if (yday < start || yday > end)
            return 0;
        if (yday > start && yday < end)
            return 1;
    } else {
        if (yday < end || yday > start)
            return 1;
        if (yday > end && yday < start)
            return 0;
    }
    ms = (int32_t)((((uint32_t)S32(tm, 8) * 60u + (uint32_t)S32(tm, 4)) * 60u + (uint32_t)S32(tm, 0)) * 1000u);
    if (yday == start)
        return ms >= GS32(0x1015c0c0);
    return ms < GS32(0x1015c0d0);
}
