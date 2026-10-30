#include "w7l.h"
#include "w3r.h"
#include "w3t.h"
#include "w4u.h"
#include "w4w.h"
#include "../util/w3o.h"

/* The runtime's truncation 0x100fcb08 (fistp of a 64-bit integer, low 32 bits; 0 for NaN and out of range). */
static int32_t W7l_Ftol(double d)
{
    if (!(d >= -9223372036854775808.0 && d < 9223372036854775808.0))
        return 0;
    return (int32_t)(uint32_t)(uint64_t)(int64_t)d;
}

// FUNCTION: HGSIM 0x1003079c
void __cdecl Sim_1003079c(void *obj)
{
    uint8_t *grid, *model;
    int i;

    if (obj == 0 || ((char)U8(obj, 0xa0) != 't' && (char)U8(obj, 0xa0) != 'T'))
        return;
    grid = (uint8_t *)Sim_1003058f();
    model = BYTES(U32(obj, 0x2c));
    if (model == 0 || grid == 0)
        return;
    U32(obj, 0x98) = (uint32_t)(uintptr_t)grid;
    Sim_10030672(obj);
    for (i = 0; i < S16(model, 6); i++) {
        uint8_t *part = model + U32(model, 8) + i * 0x28;
        float cx, cy, cz;
        uint32_t minx, maxx, miny, maxy, minz, maxz;
        int32_t gx, gz;
        uint8_t *cell;

        Sim_10030228(model, part, &cx, &cy, &cz, &minx, &maxx, &miny, &maxy, &minz, &maxz);
        gx = W7l_Ftol(((double)cx - FL(grid, 0x18) + GFL(0x101112e0)) / GFL(0x101112e4));
        gz = W7l_Ftol(((double)cz - FL(grid, 0x20) + GFL(0x101112e0)) / GFL(0x101112e4));
        if (gx < 0 || gx >= 7 || gz < 0 || gz >= 7)
            continue;
        cell = grid + gx * 0xe0 + 0x24 + gz * 0x20;
        List_PushFront(cell, part);
        if (!(Float_OfBits(minx) >= FL(cell, 0)))
            U32(cell, 0) = minx;
        if (Float_OfBits(maxx) > FL(cell, 4))
            U32(cell, 4) = maxx;
        if (!(Float_OfBits(miny) >= FL(cell, 8)))
            U32(cell, 8) = miny;
        if (Float_OfBits(maxy) > FL(cell, 0xc))
            U32(cell, 0xc) = maxy;
        if (!(Float_OfBits(minz) >= FL(cell, 0x10)))
            U32(cell, 0x10) = minz;
        if (Float_OfBits(maxz) > FL(cell, 0x14))
            U32(cell, 0x14) = maxz;
    }
}

// FUNCTION: HGSIM 0x1003129f
void __cdecl Sim_1003129f(void *obj)
{
    uint8_t *model, *p, *box;
    int i, k;

    if (G32(0x1015dd6c) != 0)
        return;
    model = BYTES(U32(obj, 0x2c));
    if (model == 0)
        return;
    p = model + 0x18;
    box = (uint8_t *)Sim_1003184b(U32(p, 0xc), U32(p, 0xc), U32(p, 0x10), U32(p, 0x10), U32(p, 0x14), U32(p, 0x14), 0);
    U32(obj, 0x98) = (uint32_t)(uintptr_t)box;
    if (box == 0)
        return;
    for (i = 1; i < S16(model, 4); i++) {
        p += 0x58;
        if (FL(p, 0xc) > FL(box, 4))
            U32(box, 4) = U32(p, 0xc);
        if (!(FL(p, 0xc) >= FL(box, 0)))
            U32(box, 0) = U32(p, 0xc);
        if (FL(p, 0x14) > FL(box, 0x14))
            U32(box, 0x14) = U32(p, 0x14);
        if (!(FL(p, 0x14) >= FL(box, 0x10)))
            U32(box, 0x10) = U32(p, 0x14);
        if (FL(p, 0x10) > FL(box, 0xc))
            U32(box, 0xc) = U32(p, 0x10);
        if (!(FL(p, 0x10) >= FL(box, 8)))
            U32(box, 8) = U32(p, 0x10);
    }
    for (k = 0; k < 4; k++)
        U32(box, 0x1c + 4 * k) = W7L_CALL_10031447(box, k, model);
}
