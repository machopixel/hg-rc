#include <string.h>

#include "w17.h"
#include "w6e.h"
#include "w10.h"
#include "../util/access.h"
#include "../util/util.h"
#include "../util/w3o.h"
#include "../unsorted_1008/w3o.h"
#include "../unsorted_1008/w8w.h"
#include "../unsorted_1002/w5z.h"
#include "../unsorted_1002/w7l.h"
#include "../unsorted_1002/w8t.h"

#define W17_C(addr) ((double)GFL(addr))
#define W17_KIND(rec) U32(U32(rec, 0), 0)

uint32_t W17_KillerLeftover(float x, float z)
{
    uint32_t ax = Bits_OfFloat(x) & 0x7fffffffu;
    uint32_t az = Bits_OfFloat(z) & 0x7fffffffu;

    return !(Float_OfBits(ax) >= Float_OfBits(az)) ? ax : az;
}

// FUNCTION: HGSIM 0x10067ec6
void __cdecl Sim_10067ec6_Credit(uint8_t *a, uint8_t *b, uint8_t *shot, uint32_t unsetKiller)
{
    int32_t killer = -2;
    int32_t part;
    double d;
    float angle;

    if (b != 0) {
        if (W17_KIND(a) == 9 && W17_KIND(b) != 9) {
            Sim_1006863d((uint32_t *)a);
            killer = S32(U32(b, 0), 4);
        } else if (W17_KIND(a) != 9 && W17_KIND(b) == 9) {
            Sim_1006863d((uint32_t *)b);
        } else {
            killer = (int32_t)unsetKiller;
        }
    }
    if (!((double)GFL(0x1015dd5c) >= W17_C(0x1012b62c))) {
        Sim_1006c161(killer, a, 0, shot, 1);
        return;
    }
    if ((double)GFL(0x1015dd5c) > W17_C(0x1012b630)) {
        FL(shot, 0x74) = (float)((double)FL(shot, 0x74) / W17_C(0x1012b604));
        Sim_1006c161(killer, a, 0, shot, 5);
        Sim_1006c161(killer, a, 0, shot, 6);
        return;
    }
    d = (double)Sim_1008486e(-GFL(0x1015dd58), -GFL(0x1015dd60));
    d = d - FL(U32(a, 0), 0x68);
    d = d - FL(U32(a, 0), 0x74);
    angle = (float)d;
    if (!(d >= W17_C(0x1012b634)))                  /* the original compares st(0) before its rounding to a float */
        angle = (float)((double)angle + W17_C(0x1012b638));
    else if ((double)angle > W17_C(0x1012b63c))
        angle = (float)((double)angle - W17_C(0x1012b638));
    if ((double)angle > W17_C(0x1012b5f8))
        part = !((double)angle >= W17_C(0x1012b640)) || (double)angle > W17_C(0x1012b644) ? 2 : 3;
    else
        part = (double)angle > W17_C(0x1012b648) || !((double)angle >= W17_C(0x1012b64c)) ? 2 : 4;
    Sim_1006c161(killer, a, 0, shot, part);
}

void __cdecl Sim_10067ec6(uint8_t *a, uint8_t *b, uint8_t *shot)
{
    Sim_10067ec6_Credit(a, b, shot, 0);
}

/* the shake of a local kind-1 unit, after the draw `roll` (0 = shake) */
static void W17_Shake(uint8_t *rec, int roll)
{
    uint32_t unit;

    if (roll != 0 || U32(rec, 0xbdb) == 5)
        return;
    unit = U32(rec, 0);
    if (U32(unit, 0) != 1)
        return;
    if (G32(0x1015ec58) != 0 && U32(unit, 4) != G32(0x1017b8c8) &&
        !(U32(unit, 0x14) == 3 && U32(unit, 0xc) == G32(0x1017b8c8)))
        return;
    U32(rec, 0xc15) = (int8_t)U8(U32(unit, 0x54), 0x59) != 0 ? 2 : 1;
}

// FUNCTION: HGSIM 0x1006788c
void __cdecl Sim_1006788c(uint8_t *a, uint8_t *b)
{
    uint8_t shot[0x158];
    int32_t rangeA, rangeB;
    float x, y, z, speed;
    int ownA = U32(U32(a, 0), 4) == G32(0x1017b8c8);
    int ownB = U32(U32(b, 0), 4) == G32(0x1017b8c8);

    if (U8(G32(0x1017b8c4), 3) == 0)
        return;
    rangeA = S32(a, 0xbdb) + (ownA ? 0x14 : 0x50);
    rangeB = S32(b, 0xbdb) + (ownB ? 0x14 : 0x50);
    x = (float)((double)FL(a, 0x554) - FL(b, 0x554));
    y = (float)((double)FL(a, 0x558) - FL(b, 0x558));
    z = (float)((double)FL(a, 0x55c) - FL(b, 0x55c));
    speed = (float)(Sim_10084a94(x, y, z) * W17_C(0x1012b614));
    memset(shot, 0, sizeof(shot));     /* the sound +0x86 is 0; the original leaves the fields it does not set unset */
    if (!((double)speed > W17_C(0x1012b618)))
        return;
    if (!((double)speed >= W17_C(0x1012b61c))) {
        U32(shot, 0x70) = 0xa;
        U32(shot, 0x74) = 0x3f000000;
        U32(shot, 0x78) = 0;
    } else if (!((double)speed >= W17_C(0x1012b620))) {
        U32(shot, 0x70) = 0x14;
        U32(shot, 0x74) = 0x3f19999a;
        U32(shot, 0x78) = 5;
        W17_Shake(a, Sim_1009c929(rangeA));
        W17_Shake(b, Sim_1009c929(rangeB));
    } else if (!((double)speed >= W17_C(0x1012b624))) {
        U32(shot, 0x70) = 0x1e;
        U32(shot, 0x74) = 0x3f333333;
        U32(shot, 0x78) = 0xa;
        W17_Shake(a, Sim_1009c929(rangeA));
        W17_Shake(b, Sim_1009c929(rangeB));
    } else if (!((double)speed >= W17_C(0x1012b628))) {
        U32(shot, 0x70) = 0x3c;
        U32(shot, 0x74) = 0x3f333333;
        U32(shot, 0x78) = 0xf;
        W17_Shake(a, Sim_1009c929(rangeA / 2));
        W17_Shake(b, Sim_1009c929(rangeB / 2));
    } else {
        U32(shot, 0x70) = 0x4b;
        U32(shot, 0x74) = 0x3f333333;
        U32(shot, 0x78) = 0x14;
        W17_Shake(a, 0);
        W17_Shake(b, 0);
    }
    U16(shot, 0x7c) = 1;
    Sim_10067ec6_Credit(a, b, shot, W17_KillerLeftover(x, z));
}

/* the hit of an object of kind 0xb0 (+2 & 0xf0): its sound / effect by the flags of its table entry (0x109e9ba0) */
static void W17_HitB0(uint8_t *a, uint8_t *obj)
{
    uint32_t entry = 0x109e9ba0u + (uint32_t)U16(obj, 0x24) * 0x50u;
    uint32_t id = U32(U32(a, 0), 4);

    U16(GBYTES(0x101654f4), 0) = (uint16_t)(U16(GBYTES(0x101654f4), 0) + 1);
    Sim_1006c161(-2, a, 0, GBYTES(0x101450a8), 5);
    Sim_1006c161(-2, a, 0, GBYTES(0x101450a8), 6);
    if ((U32(GBYTES(entry), 2) & 1) != 0)
        Sim_10024616(id, 0x24, U32(obj, 0x44), U32(obj, 0x48), U32(obj, 0x4c), U32(obj, 0x44), U32(obj, 0x48),
                     U32(obj, 0x4c));
    else if ((U32(GBYTES(entry), 2) & 2) != 0)
        Sim_10024616(id, 0x16, U32(obj, 0x44), U32(obj, 0x48), U32(obj, 0x4c), U32(obj, 0x44), U32(obj, 0x48),
                     U32(obj, 0x4c));
    Sim_1002597b(U16(obj, 0x24));
}

// FUNCTION: HGSIM 0x100680dd
void __cdecl Sim_100680dd(uint8_t *a, uint8_t *obj)
{
    uint8_t shot[0x158];
    int own = U32(U32(a, 0), 4) == G32(0x1017b8c8);
    int32_t range;
    double d;
    float speed;

    if (G32(0x10179ce0) == 0)
        return;
    range = S32(a, 0xbdb) + (own ? 0x14 : 0x50);
    if (obj != 0 && (U16(obj, 2) & 0xf0) == 0xb0) {
        W17_HitB0(a, obj);
        return;
    }
    memset(shot, 0, sizeof(shot));     /* the sound +0x86 and the kind +0x7c are 0: see w17.h */
    d = Sim_10084a94(FL(a, 0x554), FL(a, 0x558), FL(a, 0x55c)) * W17_C(0x1012b614);
    speed = (float)d;
    if (!(d > W17_C(0x1012b618)))                   /* the original compares st(0) before its rounding to a float */
        return;
    if (!((double)speed >= W17_C(0x1012b61c))) {
        U32(shot, 0x70) = 0xa;
        U32(shot, 0x74) = 0x3f000000;
        U32(shot, 0x78) = 0;
    } else if (!((double)speed >= W17_C(0x1012b620))) {
        U32(shot, 0x70) = 0x14;
        U32(shot, 0x74) = 0x3f19999a;
        U32(shot, 0x78) = 5;
        W17_Shake(a, Sim_1009c929(range));
    } else if (!((double)speed >= W17_C(0x1012b624))) {
        U32(shot, 0x70) = 0x1e;
        U32(shot, 0x74) = 0x3f333333;
        U32(shot, 0x78) = 0xa;
        W17_Shake(a, Sim_1009c929(S32(a, 0xbdb) + 0xa));
    } else if (!((double)speed >= W17_C(0x1012b628))) {
        U32(shot, 0x70) = 0x3c;
        U32(shot, 0x74) = 0x3f333333;
        U32(shot, 0x78) = 0xf;
        W17_Shake(a, Sim_1009c929(range / 2));
    } else {
        U32(shot, 0x70) = 0x4b;
        U32(shot, 0x74) = 0x3f333333;
        U32(shot, 0x78) = 0x14;
        W17_Shake(a, 0);
    }
    if (U8(G32(0x1017b8c4), 3) != 0)
        Sim_10067ec6_Credit(a, 0, shot, 0);
    if (obj != 0 && (U16(obj, 2) & 0x200) != 0)
        Sim_10025aee(U32(U32(a, 0), 4), obj, (float)(int64_t)U32(shot, 0x70), U32(obj, 0x44), U32(obj, 0x48),
                     U32(obj, 0x4c));
}
