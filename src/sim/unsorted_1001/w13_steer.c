#include "w13.h"
#include "../simmem.h"
#include "../util/access.h"
#include "../util/w3o.h"
#include "../view/view.h"
#include "../unsorted_1000/w5b.h"
#include "../unsorted_1001/w2l.h"
#include "../unsorted_1001/w3t.h"
#include "../unsorted_1001/w5a.h"
#include "../unsorted_1001/w5c.h"
#include "../unsorted_1001/w5y.h"
#include "../unsorted_1001/w7l.h"
#include "../unsorted_1001/w8t.h"
#include "../unsorted_1001/w9d.h"
#include "../unsorted_1003/fns.h"
#include "../unsorted_100c/w12.h"
#include "../unsorted_100c/w7p.h"
#include "../unsorted_100c/w8z.h"
#include "../libm/hg_math.h"

/* The C library fmod of the DLL. */
#define W13_FMOD hg_fmod

/* Without a contact: block +8 = Sim_1001412d(unit, limit) and the heading Sim_100144e9 (its answer unused). */
static void W13_SteerFree(uint8_t *unit, float limit)
{
    if (Sim_100c2ae5(unit) == 0) {
        FL(U32(unit, 0x54), 8) = Sim_1001412d(unit, limit);
        (void)Sim_100144e9(unit);
    }
}

// FUNCTION: HGSIM 0x100139ff
void __cdecl Sim_100139ff(uint8_t *unit, int16_t key)
{
    float limit, heading;
    int32_t order;

    Sim_1001403a(unit, key);
    Sim_100c264a(unit);
    limit = Sim_10014061(key);
    if (Sim_10079feb(S32(unit, 0)) != 0) {
        order = S16(unit, 0x1a6);
        if (order < 10 || order > 11)
            Sim_10017b3e(unit, 0x457a0000u); /* 4000.0f */
        else
            Sim_10017aa9(unit);
    }
    switch (S16(unit, 0x1a6)) {
    case 2:
        U8(U32(unit, 0x54), 0x26) = 0;
        U8(U32(unit, 0x54), 0x28) = 0;
        if (Sim_100c2ae5(unit) == 0) {
            FL(U32(unit, 0x54), 8) = Sim_1001412d(unit, limit);
            heading = Sim_100144e9(unit);
        } else {
            heading = Sim_10015266(unit);
        }
        Sim_1000ae00(unit, Bits_OfFloat(heading));
        break;
    case 4:
        FL(unit, 0xe8) = (float)W13_FMOD((double)FL(unit, 0xe8) + *SIM_PTR(double, 0x10111130), 360.0);
        if (S32(unit, 0) == 1 && (U16(U32(unit, 0x28), 0x562) & 1) == 0) {
            U8(U32(unit, 0x54), 0x56) = 1;
            U32(U32(unit, 0x54), 8) = 0x3c000000u; /* 1/128 */
            (void)Sim_100144e9(unit);
        } else if (Sim_100c2ae5(unit) == 0) {
            U32(U32(unit, 0x54), 8) = 0x3c800000u; /* 1/64 */
            (void)Sim_100144e9(unit);
        }
        break;
    case 5:
        Sim_100178f4(unit);
        W13_SteerFree(unit, limit);
        Sim_10015a63(unit);
        break;
    case 6:
    case 7:
    case 8:
        Sim_100178f4(unit);
        W13_SteerFree(unit, limit);
        break;
    case 13:
    case 14:
        W13_SteerFree(unit, limit);
        break;
    case 15:
        if (S32(unit, 0x23a) >= 0) {
            unsigned char *entry = Sim_10033c42(S32(unit, 0x23a));

            if (entry == 0 || Sim_100c45dc(unit, entry, (uint32_t *)(unit + 0x228)) == 2)
                Sim_10015630(unit, 5, 0x2201, 0);
        }
        break;
    default:
        if (S32(unit, 0x14) == 2 || (S32(unit, 0x14) == 3 && U32(unit, 0xc) == G32(0x1017b8c8)))
            U32(U32(unit, 0x54), 8) = 0x3c4ccccdu; /* 0.0125f */
        break;
    }
}
