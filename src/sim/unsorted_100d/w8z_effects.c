#include "w8z.h"
#include "w5c.h"
#include "w6h.h"
#include "../unsorted_1002/w7l.h"
#include "../unsorted_1004/w2j.h"
#include "../unsorted_1009/w3o.h"
#include "../util/util.h"

// FUNCTION: HGSIM 0x100d6d6a
uint32_t __cdecl Sim_100d6d6a(void *obj)
{
    uint32_t hit = 0;
    uint32_t block;
    uint8_t seg[0x44];
    uint16_t flags;
    float length;

    Sim_10047af2(BYTES(obj) + 0xbc);
    block = U32(obj, 0x28);
    /* the original works out a record address from these two fields and never uses it; the reads are kept */
    (void)*(volatile u32a *)(BYTES(block) + 0x4fc);
    (void)*(volatile u32a *)(BYTES(block) + 0x84);
    Sim_100d6f16(obj, seg);
    if (Sim_1002dda9(seg, &hit, S32(obj, 4)) != 0 && hit != 0) {
        flags = U16(hit, 2);
        if ((flags & 0x100) != 0 || (flags & 0x200) != 0) {
            length = Sim_1009cbde(seg);
            U32(obj, 0xc4) = length > W8Z_GF(0x1012bd38) ? (uint32_t)W8z_Ftol((double)length) : 0x7d0u;
            if (U32(obj, 4) == G32(0x1017b8c8))
                G32(0x1017e678) = hit;
        }
    }
    U32(obj, 0xb0) = U32(obj, 0xc4);
    Sim_10047af2(BYTES(obj) + 0xac);
    return hit;
}

/* One spread value: unchanged when it equals [0x1012bd60] (or is NaN), else random(trunc v) - trunc v / 2. */
static float W8z_Spread(float v)
{
    int32_t r;

    if (v < W8Z_GF(0x1012bd60) || v > W8Z_GF(0x1012bd60)) {
        r = Sim_1009c929(W8z_Ftol((double)v));
        r = (int32_t)((uint32_t)r - (uint32_t)(W8z_Ftol((double)v) >> 1));
        v = (float)r;
    }
    return v;
}

// FUNCTION: HGSIM 0x100d82f6
void __cdecl Sim_100d82f6(void)
{
    uint32_t e;
    uint32_t cam;
    int32_t count;
    int32_t k;
    int i;
    float sx, sy, sz;
    float x, y, z;
    uint32_t mode;

    if (G32(0x1015dc80) == 0 || G32(0x1017e684) == 0)
        return;
    for (i = 0; i < 9; i++) {
        e = (uint32_t)(uintptr_t)SIM_PTR(uint8_t, 0x10180a00) + (uint32_t)i * 0x80cu;
        if (U32(e, 0) == 0)
            continue;
        W8Z_FL(e, 0xf4) =
            (float)((double)GS32(0x1015dc80) * (double)W8Z_FL(e, 0xf8) + (double)W8Z_FL(e, 0xf4));
        count = W8z_Ftol((double)W8Z_FL(e, 0xf4));
        if (count == 0)
            continue;
        W8Z_FL(e, 0xf4) = (float)((double)W8Z_FL(e, 0xf4) - (double)count);
        mode = G32(0x1015245c);
        if (mode != 1 && mode != 2)
            continue;
        for (k = 0; k < count; k++) {
            sx = W8z_Spread(W8Z_FL(e, 0xe4));
            sy = W8z_Spread(W8Z_FL(e, 0xe8));
            sz = W8z_Spread(W8Z_FL(e, 0xec));
            cam = G32(0x1014cae0);
            z = (float)(((double)W8Z_FL(e, 0x14) + (double)sz) + (double)W8Z_FL(cam, 8));
            y = (float)(((double)W8Z_FL(e, 0x10) + (double)sy) + (double)W8Z_FL(cam, 4));
            x = (float)(((double)W8Z_FL(e, 0xc) + (double)sx) + (double)W8Z_FL(cam, 0));
            if (mode == 1)
                Sim_100d9064(x, y, z, W8Z_FL(e, 0xac), W8Z_FL(e, 0xb0), W8Z_FL(e, 0xac), (void *)(uintptr_t)e);
            else
                W8Z_CALL_100d965e(x, y, z, W8Z_FL(e, 0xac), W8Z_FL(e, 0xb0), W8Z_FL(e, 0xac), (void *)(uintptr_t)e);
        }
    }
}
