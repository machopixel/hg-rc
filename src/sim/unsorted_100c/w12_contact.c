#include "w12.h"
#include "../simmem.h"
#include "../util/access.h"
#include "../util/w3o.h"
#include "../view/view.h"
#include "../unsorted_1001/w3t.h"
#include "../unsorted_1002/w7l.h"
#include "../unsorted_1008/w2i.h"
#include "../unsorted_100c/w2f.h"
#include "../unsorted_100c/w3p.h"
#include "../unsorted_100c/w5c.h"
#include "../unsorted_100c/w6h.h"
#include "../unsorted_100c/w7p.h"
#include "../libm/hg_math.h"

/* sqrt and fabs from OpenLibm (hg_sqrt, hg_fabs), in place of the DLL's C library. */
#define W12_SQRT hg_sqrt
#define W12_FABS hg_fabs

typedef union {
    float f;
    uint32_t u;
} W12ContactBits;

/* fcomp's "equal" flag: equal, or unordered (NaN) */
static int W12_EqualOrNaN(float a, float b)
{
    return !(a < b) && !(a > b);
}

/* Sim_10079feb rejects the unit's type and Sim_100c292c reports a hit with the probe segment */
static int W12_ContactOnly(uint8_t *unit, uint8_t *seg)
{
    return Sim_10079feb(S32(unit, 0)) == 0 && Sim_100c292c(seg, unit) != 0;
}

// FUNCTION: HGSIM 0x100c2ae5
int __cdecl Sim_100c2ae5(uint8_t *unit)
{
    uint8_t seg[0x38];
    uint32_t found = 0; /* start value (DECISIONS.md): nothing hit when only Sim_100c292c reports a hit */
    float sum = 0.0f;
    int hit = 0;
    int16_t i;
    uint32_t rec = U32(unit, 0x28), block = U32(unit, 0x54);
    W12ContactBits v;

    if (W12_EqualOrNaN(FL(rec, 0x4d4), GFL(0x1012bb18)))
        return 0;
    if (S16(unit, 0x25e) == 0 && W12_EqualOrNaN(FL(block, 8), GFL(0x1012bb18)))
        return 0;
    if ((int8_t)U8(block, 0x59) != 0)
        return 0;
    if (S32(unit, 0x25a) > (int32_t)G32(0x1015dc78))
        return U32(unit, 0x24a) != 0;
    if (S16(unit, 0x25e) == 0) {
        double speed = W12_SQRT(((double)FL(rec, 0x548) * (double)FL(rec, 0x548) +
                                 (double)FL(rec, 0x54c) * (double)FL(rec, 0x54c)) +
                                (double)FL(rec, 0x550) * (double)FL(rec, 0x550));

        FL(unit, 0x260) = (float)(speed / (double)GFL(0x1012bb40));
        switch (S32(unit, 0)) {
        case 10:
            FL(unit, 0x260) = (float)((double)FL(unit, 0x260) * (double)GFL(0x1012bb88));
            break;
        case 4:
            FL(unit, 0x260) = (float)((double)FL(unit, 0x260) / (double)GFL(0x1012bb8c));
            break;
        case 1:
        case 9:
            FL(unit, 0x260) = (float)((double)FL(unit, 0x260) / (double)GFL(0x1012bb50));
            break;
        }
        if (!(FL(unit, 0x260) >= GFL(0x1012bb90)))
            U32(unit, 0x260) = 0x3e99999au;
    }
    for (i = 0; i < 4; i++) {
        float scale = (float)((double)FL(unit, 0x260) * (double)GFL(0x1012bb24));

        Sim_100c31a6(unit, seg, (int)S16(unit, 0x25e), i, scale, 0);
        if (Sim_1002dda9(seg, &found, S32(unit, 4)) == 0 && Sim_100c2851(seg, &found, U32(unit, 4), 1) == 0) {
            hit = W12_ContactOnly(unit, seg);
            if (hit == 0) {
                if (i == 0 && !((double)W12_FABS((double)Sim_10015266(unit)) > (double)GFL(0x1012bb20))) {
                    U32(unit, 0x24a) = 0;
                    U16(unit, 0x25e) = 0;
                }
                if (i == 0 && S16(unit, 0x25e) != 0) {
                    Sim_100c31a6(unit, seg, -(int)S16(unit, 0x25e), 1, scale, 1);
                    if (Sim_1002dda9(seg, &found, S32(unit, 4)) == 0 &&
                        Sim_100c2851(seg, &found, U32(unit, 4), 1) == 0) {
                        hit = W12_ContactOnly(unit, seg);
                        if (hit == 0)
                            break;
                    }
                    if (found != 0 && Sim_100c312c((const void *)(uintptr_t)found) == 0)
                        sum = (float)((double)(int32_t)S16(unit, 0x25e) * (double)GFL(0x1012bb9c) + (double)sum);
                }
                break;
            }
        }
        if (found == 0 || Sim_100c312c((const void *)(uintptr_t)found) != 0)
            break;
        if (S32(unit, 0) == 8)
            FL(unit, 0x242) = (float)((double)FL(unit, 0x242) + (double)GFL(0x1012bb10));
        if (S16(unit, 0x25e) == 0) {
            if (hit != 0) {
                U16(unit, 0x25e) = (uint16_t)Sim_100c3419(unit, FL(unit, 0x24e), FL(unit, 0x252), FL(unit, 0x256),
                                                         FL(unit, 0x58), FL(unit, 0x5c), FL(unit, 0x60));
            } else {
                U16(unit, 0x25e) = (uint16_t)Sim_100c3372(unit, (void *)(uintptr_t)found, FL(unit, 0x58),
                                                         FL(unit, 0x5c), FL(unit, 0x60));
                U32(unit, 0x24a) = found;
            }
        }
        sum = (float)((double)(int32_t)S16(unit, 0x25e) * (double)GFL(0x1012bb94) / (double)GFL(0x1012bb98) +
                      (double)sum);
    }
    if (!W12_EqualOrNaN(sum, GFL(0x1012bb18))) {
        sum = Math_ClampSymmetric_100c351c(sum, GFL(0x1012bb94));
        v.f = sum;
        U32(block, 0xc) = v.u;
        if (U32(unit, 4) == G32(0x1017b8c8)) {
            v.u = U32(unit, 0x218);
        } else {
            v.u = i == 0 ? 0x3c800000u : 0x3b800000u;
            if (hit != 0) {
                Sim_10083495();
                v.u = 0x3acccccdu;
            }
            if ((U16(rec, 0x562) & 1) != 0)
                v.u = 0;
        }
        U32(block, 8) = v.u;
        if (S32(unit, 0) == 6) {
            U32(block, 8) = 0;
            U32(block, 0xc) = 0;
        }
    } else if (S32(unit, 0) == 8) {
        if (FL(unit, 0x242) > GFL(0x1012bb34))
            FL(unit, 0x242) = (float)((double)FL(unit, 0x242) - (double)GFL(0x1012bba0));
        if ((U32(unit, 0x242) & 0x80000000u) != 0)
            U32(unit, 0x242) = 0;
    }
    Sim_100c275b(unit);
    return W12_EqualOrNaN(sum, GFL(0x1012bb18)) ? 0 : 1;
}