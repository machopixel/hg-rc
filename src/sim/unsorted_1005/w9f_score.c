#include "w9f.h"
#include "../util/access.h"

typedef float HG_MAY_ALIAS w9f_f32;
#define W9F_F32(p, off) (*(const w9f_f32 *)(BYTES(p) + (off)))

/*
 * The runtime's _ftol (0x100fcb08): x87 truncation toward zero to 64 bits, of which the low 32 are kept; out
 * of range values and NaN give the 64-bit integer indefinite, whose low half is 0.
 */
static uint32_t W9f_Ftol(double d)
{
    if (!(d >= -9223372036854775808.0 && d < 9223372036854775808.0))
        return 0;
    return (uint32_t)(uint64_t)(int64_t)d;
}

// FUNCTION: HGSIM 0x1005e426
uint32_t __cdecl Sim_1005e426(const int32_t *p, const uint8_t *recs, const int32_t *list)
{
    /* 23 entries of threshold, weight, counter (the counters start at 0) */
    int16_t table[23 * 3] = {
        0x1b58, 1, 0, 0x1b8a, 1, 0, 0x1c20, -0x3c, 0, 0x1c52, -0x3c, 0, 0x1ce8, -0x32, 0, 0x1c52, 1, 0,
        0x1c84, 1, 0, 0x1d7e, 1, 0, 0x1d7e, 1, 0, 0x1d1a, 1, 0, 0x1c84, 1, 0, 0x1db0, 1, 0, 0x1db0, 1, 0,
        0x1edc, 1, 0, 0x1d4c, 2, 0, 0x1f40, 0, 0, 0x1d1a, 2, 0, 0x1e78, 3, 0, 0x1de2, 2, 0, 0x1bee, 1, 0,
        0x1e14, 1, 0, 0x1e14, 0, 0, 0x1eaa, 0, 0,
    };
    uint16_t sum = (uint16_t)p[0];
    int32_t bumps = 0;
    int32_t acc = 0;
    uint32_t eax = 0;
    int32_t rec, j, k, i;

    for (rec = 0; rec < 0x11; rec++, recs += 0x3c) {
        for (j = 0; j < (int32_t)S16(recs, 0x34); j++) {
            eax = W9f_Ftol((double)W9F_F32(recs, 0) + (double)W9F_F32(recs, 4));
            acc += (int32_t)eax;
            for (k = 0x16; k >= 0; k--) {
                if ((int32_t)U16(recs, 0xc + j * 2) > (int32_t)table[k * 3]) {
                    table[k * 3 + 2] = (int16_t)(table[k * 3 + 2] + 1);
                    bumps++;
                    break;
                }
            }
        }
        for (k = 0x14; k >= 0; k--) {
            if (table[k * 3 + 2] != 0) {
                if (table[k * 3 + 1] < 0)
                    sum = (uint16_t)(sum + (uint16_t)((int32_t)table[k * 3 + 2] * -(int32_t)table[k * 3 + 1]));
                else
                    sum = (uint16_t)(sum + (uint16_t)((uint32_t)((int32_t)table[k * 3 + 2] * (int32_t)table[k * 3 + 1])
                                                      * (uint32_t)p[0]));
            }
        }
        if (table[0x41] != 0)
            sum = (uint16_t)(sum + (uint16_t)(bumps << 1));
        else
            sum = (uint16_t)(sum + (uint16_t)bumps);
        sum = (uint16_t)(sum + (uint16_t)acc);
        eax = W9f_Ftol((double)W9F_F32(recs, 8));
        sum = (uint16_t)(sum + (uint16_t)eax);
    }
    /* the high half of the result is the register that held the record pointer, then the list pointer */
    eax = (uint32_t)(uintptr_t)recs;
    for (i = 0; i < p[4] && i < 8; i++) {
        sum = (uint16_t)(sum + U16(GBYTES(0x101440e0), (uint32_t)(list[0] / 100) * 2u));
        list += 2;
        eax = (uint32_t)(uintptr_t)list;
    }
    return (eax & 0xffff0000u) | sum;
}
