#include "w8x.h"
#include "w2j.h"
#include "../render/w3r.h"
#include "../unsorted_1004/fns.h"
#include "../unsorted_100b/fns.h"
#include "../unsorted_100b/w6g.h"
#include "../unsorted_100f/w2e.h"
#include "../util/access.h"
#include "../util/util.h"
#include "../util/w3o.h"

typedef int(__cdecl *W8xSeenFn)(uint32_t pane, uint32_t x, uint32_t y);
typedef void(__cdecl *W8xEdgeFn)(uint32_t pane, uint32_t *x, uint32_t *y);
#define PANE_SEEN(obj) (*(W8xSeenFn *)(BYTES(obj) + 0x80))
#define PANE_EDGE(obj) (*(W8xEdgeFn *)(BYTES(obj) + 0x84))

/* The three coordinates the projection works on, also read back as the integers they are turned into. */
union W8xPoint {
    float f[3];
    uint32_t u[3];
};

/* The first 8 characters of a text, terminated (the original writes the end mark at [len + 1], len <= 7). */
static void W8x_Label(char *buf, uint32_t text)
{
    uint32_t len = Str_Length((const char *)(uintptr_t)text);

    Str_CopyN(buf, (const char *)(uintptr_t)text, 8);
    if ((int32_t)len >= 8)
        len = 7;
    buf[len + 1] = 0;
}

static int W8x_Shape(uint32_t pane, void *shape, uint32_t x, uint32_t y)
{
    if (G32(0x1017b940) != 0)
        return W8X9_CALL_100f0205((void *)(uintptr_t)pane, shape, 0, x, y);
    return W8X9_CALL_100e91dc((void *)(uintptr_t)pane, shape, 0, x, y);
}

// FUNCTION: HGSIM 0x1009a282
void __cdecl Sim_1009a282(void *obj, uint32_t x, uint32_t y, uint32_t z, char *name, int32_t a, int32_t b, uint32_t f)
{
    union W8xPoint p;
    uint32_t pane = U32(obj, 0);
    int seen;
    void *shape;

    p.u[0] = x;
    p.u[1] = y;
    p.u[2] = z;
    seen = Sim_10090099(p.f);
    if (seen != 0 && U32(obj, 0x80) != 0) {
        p.u[0] = (uint32_t)Render_Ftol((double)p.f[0]);
        p.u[1] = (uint32_t)Render_Ftol((double)p.f[1]);
        seen = PANE_SEEN(obj)(pane, p.u[0], p.u[1]);
    }
    if (seen == 0)
        return;
    if (b > 4)
        b = 4;
    if (b > 0) {
        double limit = (double)Float_OfBits(f) / (double)GFL(0x1012b9c0);

        if (!((double)U32(GBYTES(0x1014be6d), (uint32_t)b * 0x1d) >= limit))
            return;
    }
    if (a == 1)
        G32(0x1017cc5c) = 1;
    shape = W8X9_CALL_100ba7d5(name, (void *)(uintptr_t)G32(0x1014cec0), 0);
    if (shape == 0)
        return;
    W8x_Shape(pane, shape, p.u[0], p.u[1]);
    Sim_100bb2b0((const uint32_t *)name, (const char *)(uintptr_t)G32(0x1014cec0));
}

// FUNCTION: HGSIM 0x1009a3b3
void __cdecl Sim_1009a3b3(void *obj)
{
    uint32_t me = PTRTAB(G32(0x1017b8c8));
    uint32_t mine = U32(me, 0x28);
    uint32_t pos[3];
    char label[12];
    int32_t i;

    G32(0x1017cc5c) = 0;
    for (i = 0; i < GS32(0x1015dc64); i++) {
        uint32_t o, other;
        int16_t flags;
        int k;

        if (i == GS32(0x1017b8c8))
            continue;
        o = PTRTAB(i);
        other = U32(PTRTAB(i), 0x28);
        flags = S16(o, 0x1c);
        if (!(flags & 0x1400) || (flags & 0x800) || (flags & 0x4000) || (flags & 0x16))
            continue;
        pos[0] = U32(o, 0x58);
        pos[1] = U32(o, 0x5c);
        pos[2] = U32(o, 0x60);
        k = Sim_10052be7(i);
        W8x_Label(label, U32(U32(obj, 0x70), (uint32_t)k * 4 + 0xc));
        if (k == 1) {
            uint32_t r0, r1, r2, r3;

            W8X9_CALL_100cbc6b((float)((double)FL(o, 0x58) - (double)FL(me, 0x58)),
                               (float)((double)FL(o, 0x5c) - (double)FL(me, 0x5c)),
                               (float)((double)FL(o, 0x60) - (double)FL(me, 0x60)), &r0, &r1, &r2, &r3);
            Sim_1009a282(obj, pos[0], pos[1], pos[2], label, k, (int32_t)(U32(other, 0xbdf) - U32(mine, 0xbe3)), r1);
        } else {
            Sim_1009a282(obj, pos[0], pos[1], pos[2], label, k, 0, 0);
        }
    }
    for (i = 0; i < GS32(0x1015dc68); i++) {
        uint8_t *e = GBYTES(0x109e9ba0) + i * 0x50;
        int k;

        if (!(S16(e, 0) & 0x1400) || (S16(e, 0) & 0x1e))
            continue;
        Sim_100b53f0((int)U32(e, 8), &pos[0], &pos[1], &pos[2]);
        k = Sim_10052c02(i);
        W8x_Label(label, U32(U32(obj, 0x70), (uint32_t)k * 4 + 0xc));
        Sim_1009a282(obj, pos[0], pos[1], pos[2], label, k, 0, 0);
    }
}

// FUNCTION: HGSIM 0x1009a6f3
void __cdecl Sim_1009a6f3(void *obj)
{
    union W8xPoint p;
    uint32_t pane = U32(obj, 0), me, target;
    char label[12];
    int left = 2, seen;

    if (G32(0x1017cca0) != 0)
        pane = G32(0x1017cca0);
    me = PTRTAB(G32(0x1017b8c8));
    p.u[0] = U32(me, 0xdc);
    p.u[1] = U32(me, 0xe0);
    p.u[2] = U32(me, 0xe4);
    seen = Sim_10090099(p.f);
    p.u[0] = (uint32_t)Render_Ftol((double)p.f[0]);
    p.u[1] = (uint32_t)Render_Ftol((double)p.f[1]);
    if (seen != 0 && U32(obj, 0x80) != 0)
        seen = PANE_SEEN(obj)(pane, p.u[0], p.u[1]);
    target = U32(me, 0xf0);
    while (left != 0) {
        if (target != 0 && !(target & 0x1000)) {
            uint32_t idx = target & 0xff, text;
            int sel, row;
            void *shape;

            switch (target & 0xf00) {
            case 0x200:
                sel = Sim_10052be7((int)idx);
                row = ((S16(PTRTAB(idx), 0x1c) & 0x1400) && !(S16(PTRTAB(idx), 0x1c) & 0x800)) ? 2 : 3;
                break;
            case 0x400:
                sel = Sim_10052c02((int)idx);
                row = (S16(GBYTES(0x109e9ba0) + idx * 0x50, 0) & 0x1400) ? 2 : 3;
                break;
            case 0x100:
                if (seen != 0)
                    sel = Sim_1009a6b3(GBYTES(0x109a8640) + U32(me, 0x118) * 0x54, (int)G32(0x1013bb40)) ? 1 : 0;
                else
                    sel = 0;
                row = 5;
                break;
            default:
                return;
            }
            if (seen == 0)
                row = 6;
            text = U32(U32(obj, 0x70) + (uint32_t)row * 0xc, (uint32_t)sel * 4);
            if (text == 0)
                return;
            W8x_Label(label, text);
            shape = W8X9_CALL_100ba7d5(label, (void *)(uintptr_t)G32(0x1014cec0), 0);
            if (shape != 0) {
                if (seen != 0) {
                    W8x_Shape(pane, shape, p.u[0], p.u[1]);
                } else if (U32(obj, 0x84) != 0) {
                    PANE_EDGE(obj)(pane, &p.u[0], &p.u[0]);
                    W8x_Shape(pane, shape, p.u[0], p.u[1]);
                }
                Sim_100bb2b0((const uint32_t *)label, (const char *)(uintptr_t)G32(0x1014cec0));
            }
        }
        /* the second marker: its point is handed on as the projected floats, not truncated */
        p.u[0] = U32(me, 0x104);
        p.u[1] = U32(me, 0x108);
        p.u[2] = U32(me, 0x10c);
        seen = Sim_10090099(p.f);
        if (seen != 0 && U32(obj, 0x80) != 0)
            seen = PANE_SEEN(obj)(pane, p.u[0], p.u[1]);
        target = U32(me, 0x118);
        left--;
    }
}

// FUNCTION: HGSIM 0x1009aac9
void __cdecl Sim_1009aac9(void *obj)
{
    uint32_t pane = U32(obj, 0);
    char label[12];
    int32_t i;

    if (G32(0x1017cca0) != 0)
        pane = G32(0x1017cca0);
    for (i = 0; i < GS32(0x1017e5a8); i++) {
        uint8_t *e = GBYTES(0x109a8640) + i * 0x54;
        union W8xPoint p;
        uint32_t text;
        void *shape;
        int seen;

        if (U32(e, 0xc) != G32(0x1013bb40) || U32(e, 0) == 0 || (S16(e, 0x24) & 1))
            continue;
        if (U32(e, 4) != 0) {
            Sim_100a9cfb(BYTES(U32(e, 4)), &p.u[0], &p.u[1], &p.u[2]);
        } else {
            p.u[0] = U32(e, 0x18);
            p.u[1] = U32(e, 0x1c);
            p.u[2] = U32(e, 0x20);
        }
        seen = Sim_10090099(p.f);
        p.u[0] = (uint32_t)Render_Ftol((double)p.f[0]);
        p.u[1] = (uint32_t)Render_Ftol((double)p.f[1]);
        if (seen != 0 && U32(obj, 0x80) != 0)
            seen = PANE_SEEN(obj)(pane, p.u[0], p.u[1]);
        if (seen == 0)
            continue;
        if (Sim_1009a6b3(e, (int)G32(0x1013bb40)) != 0)
            text = U32(U32(obj, 0x70), 0x34);
        else
            text = U32(U32(obj, 0x70), 0x30);
        W8x_Label(label, text);
        shape = W8X9_CALL_100ba7d5(label, (void *)(uintptr_t)G32(0x1014cec0), 0);
        if (shape == 0)
            continue;
        W8x_Shape(pane, shape, p.u[0], p.u[1]);
        Sim_100bb2b0((const uint32_t *)label, (const char *)(uintptr_t)G32(0x1014cec0));
    }
}
