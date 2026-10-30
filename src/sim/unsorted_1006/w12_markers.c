#include "w12.h"
#include "../simmem.h"
#include "../util/access.h"
#include "../util/w3o.h"
#include "../render/w2n.h"
#include "../unsorted_1000/w3o.h"
#include "../unsorted_1006/w7n.h"
#include "../unsorted_1006/w8w.h"
#include "../unsorted_1008/w6f.h"
#include "../unsorted_100d/w8z.h"
#include "../unsorted_100f/w2e.h"

typedef union {
    float f;
    uint32_t u;
} W12MarkBits;

static float W12_MarkF(uint32_t u)
{
    W12MarkBits b;

    b.u = u;
    return b.f;
}

static uint32_t W12_Palette(int32_t p)
{
    return G32(0x1017b940) != 0 ? 0x10176960u + ((uint32_t)p << 9) : 0x10178b60u + ((uint32_t)p << 8);
}

static void W12_Draw(float x, float y, char *name, int32_t palette)
{
    Sim_10065216((uint32_t)Render_Ftol((double)x), (uint32_t)Render_Ftol((double)y), name, W12_Palette(palette));
}

/* pos: the point as float bits (x, y, z); size: float bits */
static void W12_Marker(uint32_t *pos, uint32_t size, uint32_t id, int32_t kind)
{
    char text[16], below[16], right[16], left[16];
    int32_t palette = 0; /* start value (DECISIONS.md): on screen, for a kind other than 0..2 */
    float x, y, d;

    if (Sim_100d2669((float *)&pos[0], (float *)&pos[1], (float *)&pos[2]) == 0) {
        int32_t pt[2];

        pt[0] = Render_Ftol((double)W12_MarkF(pos[0]));
        pt[1] = Render_Ftol((double)W12_MarkF(pos[1]));
        Sim_1008e266(SIM_PTR(const int32_t, 0x109e3e00), pt, pt);
        palette = kind == 0 ? 10 : kind == 2 ? 11 : 9;
        if (pt[0] == 0)
            Str_Copy(text, (const char *)(uintptr_t)G32(0x109e85a4));
        else if (pt[1] == 0)
            Str_Copy(text, (const char *)(uintptr_t)G32(0x109e85ac));
        else if (pt[0] == (int32_t)G32(0x109e3e0c) + 1)
            Str_Copy(text, (const char *)(uintptr_t)G32(0x109e85a8));
        else
            Str_Copy(text, (const char *)(uintptr_t)G32(0x109e85a0));
        Sim_10063415(1, id, (uint32_t)Render_Ftol((double)W12_MarkF(pos[0])),
                     (uint32_t)Render_Ftol((double)W12_MarkF(pos[1])), kind);
        Sim_10065216((uint32_t)pt[0], (uint32_t)pt[1], text, W12_Palette(palette));
        return;
    }
    d = (float)((double)FL(G32(0x1014cae0), 0x94) * (double)W12_MarkF(size) / (double)W12_MarkF(pos[2]));
    if ((double)(int32_t)G32(0x1015dc38) > (double)d)
        d = (float)(int32_t)G32(0x1015dc38);
    Str_Copy(text, (const char *)(uintptr_t)G32(0x109e85b0));
    Str_Copy(below, (const char *)(uintptr_t)G32(0x109e85bc));
    Str_Copy(right, (const char *)(uintptr_t)G32(0x109e85b8));
    Str_Copy(left, (const char *)(uintptr_t)G32(0x109e85b4));
    if (kind == 0)
        palette = 1;
    else if (kind == 1)
        palette = 0;
    else if (kind == 2)
        palette = 2;
    if (Sim_10063415(0, id, (uint32_t)Render_Ftol((double)W12_MarkF(pos[0])),
                     (uint32_t)Render_Ftol((double)W12_MarkF(pos[1])), kind) == 0)
        return;
    x = W12_MarkF(pos[0]);
    y = W12_MarkF(pos[1]);
    W12_Draw(x, (float)((double)y - (double)d), text, palette);
    W12_Draw(x, (float)((double)y + (double)d), below, palette);
    W12_Draw((float)((double)x + (double)d), y, right, palette);
    W12_Draw((float)((double)x - (double)d), y, left, palette);
}

// FUNCTION: HGSIM 0x100648a0
void __cdecl Sim_100648a0(const uint8_t *unit, int32_t kind)
{
    uint32_t rec = U32(unit, 0x28);
    uint32_t pos[3];

    pos[0] = U32(unit, 0x58);
    if ((U16(rec, 0x562) & 8) != 0) {
        W12MarkBits b;

        b.f = (float)((double)FL(unit, 0x5c) - (double)FL(rec, 0x520));
        pos[1] = b.u;
    } else {
        pos[1] = U32(unit, 0x5c);
    }
    pos[2] = U32(unit, 0x60);
    W12_Marker(pos, U32(rec, 0x53c), U32(unit, 4), kind);
}

// FUNCTION: HGSIM 0x10064d05
void __cdecl Sim_10064d05(uint32_t id, const uint8_t *thing, int32_t kind)
{
    uint32_t pos[3];
    W12MarkBits size;

    if (thing == 0 || U32(thing, 0x8c) == 0)
        return;
    size.f = Sim_1000946e((void *)(uintptr_t)U32(thing, 0x8c), &pos[0], &pos[1], &pos[2]);
    W12_Marker(pos, size.u, id, kind);
}