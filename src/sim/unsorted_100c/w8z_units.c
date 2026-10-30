#include "w8z.h"
#include "fns.h"
#include "w2e.h"
#include "w2f.h"
#include "w2l.h"
#include "w3p.h"
#include "w3t.h"
#include "w5c.h"
#include "w5z.h"
#include "w7p.h"
#include "../unsorted_1000/w5b.h"
#include "../unsorted_1001/w2e.h"
#include "../unsorted_1001/w3t.h"
#include "../unsorted_1001/w5c.h"
#include "../unsorted_1001/w5y.h"
#include "../unsorted_1001/w6d.h"
#include "../unsorted_1008/w7n.h"
#include "../unsorted_1003/fns.h"
#include "../util/util.h"

/* The record of the unit (+0x54) and its float +8. */
static void W8z_SetRecordFloat(void *unit, float value)
{
    W8Z_FL(U32(unit, 0x54), 8) = value;
}

// FUNCTION: HGSIM 0x100c0670
void __cdecl Sim_100c0670(void *unit, short key)
{
    float speed;
    float limit;
    float value;

    Sim_1001403a(unit, key);
    if (W8Z_CALL_100c2ae5(unit) == 0) {
        value = W8Z_CALL_1001412d(unit, Sim_100c05cb(unit));
        W8z_SetRecordFloat(unit, value);
        speed = Sim_100144e9(unit);
        Sim_100c0379(unit, speed);
    } else {
        speed = Sim_10015266(unit);
    }
    if (!((double)S32(unit, 0x278) >= (double)W8Z_GF(0x1012bb2c)))
        limit = (float)S32(unit, 0x278);
    else
        limit = W8z_Float(0x458ca000);
    if (!(W8Z_FL(unit, 0xd4) >= limit))
        U32(unit, 0x1fc) = 10;
    if (Sim_1000ae00(unit, W8z_Bits(speed)) == 0 && U32(unit, 0) == 1 && W8Z_CALL_1000a6f0(unit, 1) == 0) {
        if (!((double)S32(unit, 0x278) >= (double)W8Z_GF(0x1012bb2c)))
            limit = (float)S32(unit, 0x278);
        else
            limit = W8z_Float(0x458ca000);
        value = W8Z_CALL_1001412d(unit, (float)((double)limit + (double)W8Z_GF(0x1012bb30)));
        W8z_SetRecordFloat(unit, value);
    }
}

// FUNCTION: HGSIM 0x100c07c5
void __cdecl Sim_100c07c5(void *unit, short unused)
{
    float speed;
    float value;

    (void)unused;
    Sim_1001403a(unit, S16(unit, 0x1aa));
    speed = Sim_10015266(unit);
    Sim_1000ae00(unit, W8z_Bits(speed));
    if (U32(unit, 0x21c) == 0) {
        Sim_1001403a(unit, S16(unit, 0x1a8));
        if (W8Z_CALL_100c2ae5(unit) == 0) {
            Sim_100144e9(unit);
            value = W8Z_CALL_1001412d(unit, W8z_Float(0x457a0000));
            W8z_SetRecordFloat(unit, value);
        }
        if (!(Crt_Fabs((double)W8Z_FL(U32(unit, 0x54), 8) - (double)W8Z_GF(0x1012bb18)) >=
              (double)W8Z_GF(0x1012bb34)) &&
            Crt_Fabs((double)speed) > W8Z_GD(0x1012bb38))
            U32(unit, 0x21c) = 1;
    }
    if (U32(unit, 0x21c) == 1) {
        if (W8Z_CALL_100c2ae5(unit) != 0)
            value = Sim_10015266(unit);
        else
            value = Sim_100144e9(unit);
        if (Sim_100c0379(unit, value) == 0)
            U32(U32(unit, 0x54), 8) = 0;
        Sim_1001403a(unit, S16(unit, 0x1a8));
        if (W8Z_FL(unit, 0xd4) > W8Z_GF(0x1012bb2c))
            U32(unit, 0x21c) = 0;
    }
}

// FUNCTION: HGSIM 0x100c0965
int __cdecl Sim_100c0965(void *unit, short key)
{
    int result = 0;
    float speed;
    float value;
    double rate;
    float factor;

    Sim_1001403a(unit, key);
    if (W8Z_CALL_100c2ae5(unit) == 0) {
        speed = Sim_100144e9(unit);
        value = W8Z_CALL_1001412d(unit, W8z_Float(0x45abe000));
        W8z_SetRecordFloat(unit, value);
    } else {
        speed = Sim_10015266(unit);
    }
    Sim_1000ae00(unit, W8z_Bits(speed));
    rate = (double)Sim_100c3725(unit) / (double)W8Z_GF(0x1012bb40);
    factor = (float)rate;
    if (!(rate >= (double)W8Z_GF(0x1012bb20)))
        factor = 1.0f;
    if ((double)factor * (double)W8Z_GF(0x1012bb44) >= (double)W8Z_FL(unit, 0xd4))
        result = 1;
    return result;
}

// FUNCTION: HGSIM 0x100c0a2d
int __cdecl Sim_100c0a2d(void *unit, short key)
{
    int result = 0;
    float value;

    Sim_1001403a(unit, key);
    if (W8Z_CALL_100c2ae5(unit) == 0 || Sim_100c38fe((uint16_t)key) == U32(unit, 0x24a)) {
        Sim_100144e9(unit);
        value = W8Z_CALL_1001412d(unit, 0.0f);
        W8z_SetRecordFloat(unit, value);
    }
    if (!(W8Z_FL(unit, 0xd8) > W8Z_GF(0x1012bb48))) {
        U8(U32(unit, 0x54), 0x4c) = 1;
        result = 1;
    }
    return result;
}

// FUNCTION: HGSIM 0x100c0b1d
int __cdecl Sim_100c0b1d(void *unit, short unused)
{
    float value;

    (void)unused;
    Sim_1001403a(unit, S16(unit, 0x1aa));
    value = Sim_10015266(unit);
    Sim_1000ae00(unit, W8z_Bits(value));
    Sim_1001403a(unit, S16(unit, 0x1a8));
    if (W8Z_CALL_100c2ae5(unit) == 0) {
        value = W8Z_CALL_1001412d(unit, W8z_Float(0x453b8000));
        W8z_SetRecordFloat(unit, value);
        Sim_100144e9(unit);
    }
    return W8Z_FL(unit, 0xd4) > W8Z_GF(0x1012bb4c) ? 0 : 1;
}

// FUNCTION: HGSIM 0x100c1049
int __cdecl Sim_100c1049(void *unit, short unused)
{
    float value;

    (void)unused;
    Sim_1001403a(unit, S16(unit, 0x1aa));
    if (GS32(0x1015dc78) > S32(unit, 0x204)) {
        if ((double)W8Z_FL(unit, 0xd4) > W8Z_GD(0x1012bb58)) {
            Sim_1001570f(unit);
            Sim_100157ca(unit);
        }
        U32(unit, 0x204) = G32(0x1015dc78) + 0x5a8u;
    }
    value = Sim_10015266(unit);
    Sim_1000ae00(unit, W8z_Bits(value));
    Sim_1001403a(unit, S16(unit, 0x1a8));
    if (W8Z_CALL_100c2ae5(unit) == 0) {
        value = W8Z_CALL_1001412d(unit, W8z_Float(0x453b8000));
        W8z_SetRecordFloat(unit, value);
        Sim_100144e9(unit);
    }
    if (!(W8Z_FL(unit, 0xd4) > W8Z_GF(0x1012bb4c)))
        Sim_100159cf(unit, S16(unit, 0x1a8));
    return 0;
}

// FUNCTION: HGSIM 0x100c1169
int __cdecl Sim_100c1169(void *unit, short unused)
{
    float value;

    (void)unused;
    Sim_1001403a(unit, S16(unit, 0x1aa));
    value = Sim_10015266(unit);
    Sim_1000ae00(unit, W8z_Bits(value));
    Sim_1001403a(unit, S16(unit, 0x1a8));
    if (W8Z_CALL_100c2ae5(unit) == 0) {
        value = W8Z_CALL_1001412d(unit, W8z_Float(0x453b8000));
        W8z_SetRecordFloat(unit, value);
        Sim_100144e9(unit);
    }
    if (!(W8Z_FL(unit, 0xd4) > W8Z_GF(0x1012bb4c))) {
        Sim_1001570f(unit);
        if (U16(unit, 0x1aa) == U16(unit, 0x1a8)) {
            if ((U16(unit, 0x1aa) & 0x200) != 0)
                Sim_100c00c7(unit, U16(unit, 0x1aa), S16(unit, 0x218), 0x46afc800u);
            else
                Sim_100c00c7(unit, U32(unit, 4) | 0x200u, 1, 0x46afc800u);
        } else {
            Sim_1001403a(unit, S16(unit, 0x1aa));
            U16(unit, 0x1a8) = U16(unit, 0x1aa);
        }
    }
    return 0;
}

// FUNCTION: HGSIM 0x100c12d7
int __cdecl Sim_100c12d7(void *unit)
{
    int result = 0;
    float value;
    int entry;
    int step;

    Sim_1001403a(unit, S16(unit, 0x1aa));
    value = Sim_10015266(unit);
    Sim_1000ae00(unit, W8z_Bits(value));
    switch (U32(unit, 0x218)) {
    case 1:
        if (GS32(0x109e4cc0) <= 0) {
            result = -1;
            break;
        }
        entry = Sim_100c4035(unit, (void *)(uintptr_t)PTRTAB(U16(unit, 0x1aa) & 0xff), 0x46dac000u);
        if (entry >= 0) {
            U32(unit, 0x218) = 2;
            U32(unit, 0x23a) = (uint32_t)entry;
            result = 0;
        } else {
            result = -1;
        }
        break;
    case 2:
        step = Sim_100c45dc(unit, Sim_10033c42(S32(unit, 0x23a)), &U32(unit, 0x220));
        if (step == 1)
            U32(unit, 0x218) = 3;
        else if (step == 2)
            U32(unit, 0x218) = 1;
        else
            U32(unit, 0x218) = 2;
        break;
    case 3:
        result = 1;
        break;
    default:
        result = -1;
        break;
    }
    return result;
}

// FUNCTION: HGSIM 0x100c18c8
int __cdecl Sim_100c18c8(void *unit)
{
    float value;
    void *entry;

    if (S32(unit, 0x23a) < 0)
        return -1;
    if (Sim_100c4d63(unit) != 0) {
        Sim_100c48c9(unit);
        Sim_1001403a(unit, S16(unit, 0x1aa));
        value = Sim_10015266(unit);
        Sim_1000ae00(unit, W8z_Bits(value));
        return -1;
    }
    switch (U32(unit, 0x218) & 0xf) {
    case 1:
        Sim_100c48c9(unit);
        entry = Sim_10033c42(S32(unit, 0x23a));
        switch (U32(unit, 0x218) & 0xf0) {
        case 0x10:
            if (Sim_100c4e1c(unit, entry, &S32(unit, 0x224)) != 0) {
                U32(unit, 0x218) |= 0x20;
                U32(unit, 0x218) &= ~0x10u;
            } else {
                U32(unit, 0x218) |= 0x10;
                U32(unit, 0x218) &= ~0x20u;
            }
            break;
        case 0x20:
            Sim_1001403a(unit, S16(unit, 0x1aa));
            value = Sim_100144e9(unit);
            if (Sim_1000ae00(unit, W8z_Bits(value)) != 0)
                U32(unit, 0x21c) += 1;
            break;
        }
        if (S32(unit, 0x21c) >= 8 || GS32(0x1015dc78) >= S32(unit, 0x204)) {
            U32(unit, 0x21c) = 0;
            U32(unit, 0x204) = G32(0x1015dc78) + 0x43eu;
            U32(unit, 0x218) |= 2;
            U32(unit, 0x218) &= ~1u;
            U32(unit, 0x218) &= 0xffffff0fu;
            U32(unit, 0x218) |= 0x10;
        }
        break;
    case 2:
        entry = Sim_10033c42(S32(unit, 0x23a));
        switch (U32(unit, 0x218) & 0xf0) {
        case 0x10:
            if (Sim_100c45dc(unit, entry, &U32(unit, 0x228)) == 1) {
                U32(unit, 0x218) &= 0xffffff0fu;
                U32(unit, 0x218) |= 0x20;
            }
            break;
        case 0x20:
            Sim_100c4898(unit);
            break;
        }
        if (GS32(0x1015dc78) >= S32(unit, 0x204)) {
            U32(unit, 0x21c) = 0;
            U32(unit, 0x204) = G32(0x1015dc78) + 0x5a8u;
            U32(unit, 0x218) |= 1;
            U32(unit, 0x218) &= ~2u;
            U32(unit, 0x218) &= 0xffffff0fu;
            U32(unit, 0x218) |= 0x10;
            Sim_100c48c9(unit);
        }
        break;
    }
    return 0;
}

// FUNCTION: HGSIM 0x100c2279
int __cdecl Sim_100c2279(void *unit, short unused)
{
    int result = 0;
    uint16_t partnerKey = U16(unit, 0x1aa);
    int16_t key;
    float value;
    uint32_t partner;
    float dx, dy, dz;
    float sx, sy, sz;
    float px, py, pz;

    (void)unused;
    Sim_1001403a(unit, (short)partnerKey);
    value = Sim_10015266(unit);
    Sim_1000ae00(unit, W8z_Bits(value));
    if (U32(unit, 0x218) == 1) {
        key = S16(unit, 0x1a8);
        if ((key & 0x100) != 0) {
            Sim_100ca417(U32(unit, 4), (uint32_t)(int32_t)key);
            U16(unit, 0x1a8) = U16(unit, 0x1aa);
        }
        partner = PTRTAB((uint32_t)(int32_t)(int16_t)partnerKey & 0xff);
        dx = (float)((double)W8Z_FL(unit, 0x58) - (double)W8Z_FL(partner, 0x58));
        dy = (float)((double)W8Z_FL(unit, 0x5c) - (double)W8Z_FL(partner, 0x5c));
        dz = (float)((double)W8Z_FL(unit, 0x60) - (double)W8Z_FL(partner, 0x60));
        sx = 0.0f;
        sy = Sim_1009c929(2) != 0 ? 1.0f : -1.0f;
        sz = 0.0f;
        /* the cross product side x d, rounded to float like the original's three stores */
        px = (float)((double)dz * (double)sy - (double)dy * (double)sz);
        py = (float)((double)dx * (double)sz - (double)dz * (double)sx);
        pz = (float)((double)dy * (double)sx - (double)dx * (double)sy);
        Sim_10084cee(&px, &py, &pz);
        px = (float)((double)px * (double)W8Z_GF(0x1012bb44));
        py = (float)((double)py * (double)W8Z_GF(0x1012bb44));
        pz = (float)((double)pz * (double)W8Z_GF(0x1012bb44));
        px = (float)((double)W8Z_FL(partner, 0x58) + (double)px);
        py = (float)((double)W8Z_FL(partner, 0x5c) + (double)py);
        pz = (float)((double)W8Z_FL(partner, 0x60) + (double)pz);
        key = (int16_t)Sim_100ca350(U32(unit, 4), W8z_Bits(px), W8z_Bits(py), W8z_Bits(pz));
        if (key != -1) {
            Sim_100159cf(unit, (int16_t)((int32_t)key | 0x100));
            U32(unit, 0x218) = 2;
        } else {
            result = -1;
        }
    } else if (U32(unit, 0x218) == 2) {
        if (W8Z_CALL_100c2ae5(unit) == 0) {
            Sim_1001403a(unit, S16(unit, 0x1a8));
            Sim_100144e9(unit);
            value = W8Z_CALL_1001412d(unit, W8z_Float(0x44fa0000));
            W8z_SetRecordFloat(unit, value);
            if (!(W8Z_FL(unit, 0xd4) > W8Z_GF(0x1012bb48)))
                U32(unit, 0x218) = 1;
        } else {
            result = -1;
            Sim_100ca417(U32(unit, 4), U16(unit, 0x1a8));
            U16(unit, 0x1a8) = U16(unit, 0x1aa);
        }
    } else {
        result = -1;
        Sim_100ca417(U32(unit, 4), U16(unit, 0x1a8));
        U16(unit, 0x1a8) = U16(unit, 0x1aa);
    }
    return result;
}

// FUNCTION: HGSIM 0x100c45dc
int __cdecl Sim_100c45dc(void *unit, void *entry, uint32_t *slot)
{
    int result = 0;
    float value;

    if (Sim_100c459f(entry, unit) == 0)
        return 2;
    Sim_100c4498(unit, (uint32_t)(uintptr_t)entry, slot);
    Sim_1001403a(unit, S16(unit, 0x1a8));
    if (W8Z_CALL_100c2ae5(unit) == 0) {
        Sim_100144e9(unit);
        value = W8Z_CALL_1001412d(unit, W8z_Float(0x447a0000));
        W8z_SetRecordFloat(unit, value);
    }
    if (!(W8Z_FL(unit, 0xd4) > W8Z_GF(0x1012bb10))) {
        U32(U32(unit, 0x54), 8) = 0;
        result = 1;
    }
    return result;
}

// FUNCTION: HGSIM 0x100c4e1c
int __cdecl Sim_100c4e1c(void *unit, void *entry, int32_t *slot)
{
    int result;
    int dir = 0;
    int32_t span = 0;
    float value;

    (void)entry;
    Sim_100c48c9(unit);
    result = W8Z_CALL_1000a6f0(unit, 1);
    Sim_1001403a(unit, S16(unit, 0x1aa));
    value = Sim_100144e9(unit);
    if (result != 0)
        return result;
    if (Crt_Fabs((double)value) >= (double)W8Z_GF(0x1012bb60))
        return result;
    if (*slot < 0) {
        dir = 0;
        span = (int32_t)(0u - (uint32_t)*slot);
    } else if (*slot > 0) {
        dir = 1;
        span = *slot;
    } else {
        *slot = (int32_t)(G32(0x1015dc78) + 0x16au);
        span = *slot;
        dir = Sim_1009c929(2);
        *slot = (int32_t)((uint32_t)*slot * (dir != 0 ? 1u : 0xffffffffu));
    }
    Sim_100c48fa(unit, dir);
    if (GS32(0x1015dc78) > span) {
        result = 1;
        *slot = 0;
        Sim_100c48fa(unit, 2);
    } else {
        result = 0;
    }
    return result;
}
