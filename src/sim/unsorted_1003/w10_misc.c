#include "w10.h"
#include "../simmem.h"
#include <string.h>
#include "w3o.h"
#include "w4w.h"
#include "fns.h"
#include "../unsorted_1002/w7l.h"
#include "../unsorted_1009/w2e.h"
#include "../util/access.h"
#include "../unsorted_1009/w3o.h"
#include "../unsorted_1009/w7o.h"
#include "../render/w2n.h"
#include "../unsorted_100d/w8z.h"
#include "../unsorted_100f/w2e.h"
#include "../unsorted_100f/w6h.h"
#include "../unsorted_100f/w6i.h"
#include "../unsorted_100f/w8z.h"
#include "../unsorted_1004/w3p.h"

typedef union {
    float f;
    uint32_t u;
} W10_3Word;

// FUNCTION: HGSIM 0x10034111
int __cdecl Sim_10034111(uint32_t unused, void *seg, const float *box)
{
    const float *s = (const float *)seg;
    W10_3Word nearT, farT, n, f;
    float len;

    (void)unused;
    if (box == 0)
        return 0;
    if (Sim_1009cfd4(s[0], s[9], box[0], box[1], &nearT.u, &farT.u) != 0)
        return 0;
    if (Sim_1009cfd4(s[1], s[10], box[2], box[3], &n.u, &f.u) != 0)
        return 0;
    /* fcomp: an unordered compare takes the "below or equal" side */
    if (n.f > nearT.f)
        nearT.u = n.u;
    if (!(f.f >= farT.f))
        farT.u = f.u;
    if (Sim_1009cfd4(s[2], s[11], box[4], box[5], &n.u, &f.u) != 0)
        return 0;
    if (n.f > nearT.f)
        nearT.u = n.u;
    if (!(f.f >= farT.f))
        farT.u = f.u;
    if (nearT.f > farT.f)
        return 0;
    len = Sim_1009cbde(seg);
    if (farT.f > len)
        farT.f = len;
    if (nearT.f > farT.f)
        return 0;
    Sim_1009cd36(seg, nearT.f);
    return 1;
}

typedef int(__cdecl *W10_3FilterFn)(void *obj);
typedef void(__cdecl *W10_3BoxFn)(void *target, int32_t x, int32_t y, int32_t r, int32_t w, uint32_t colour);
typedef void *(__cdecl *W10_3LoadFn)(void *name, void *key, uint32_t zero);
typedef void(__cdecl *W10_3TextFn)(void *target, int32_t x, int32_t y, void *font, const char *text, void *colours);

#define W10_3NAME ((const char *)GBYTES(0x1015e508))

// FUNCTION: HGSIM 0x100357db
void __cdecl Sim_100357db(uint8_t *list)
{
    const float *cam;
    uint32_t e, r, other;
    float x, y, z, rad;
    uint32_t colour;
    int32_t w;
    void *f;

    if (list == 0 || U32(list, 8) == 0 || U32(list, 8) == U32(list, 4))
        return;
    if (G32(0x1015e554) == 0 || G32(0x10179cc0) != 0) {
        G32(0x10179cc0) = 0;
        Str_Copy((char *)GBYTES(0x1015e508), SIM_PTR(const char, 0x1013a038));          /* "TRHP" */
        f = Sim_100fd850(SIM_PTR(const char, 0x1013a044), SIM_PTR(const char, 0x1013a040));  /* object.dat, r */
        if (f != 0) {
            Sim_100fe450((uint32_t)(uintptr_t)f, (uint32_t)(uintptr_t)SIM_PTR(const char, 0x1013a050),
                         (uint32_t)(uintptr_t)GBYTES(0x1015e508));
            Crt_FClose(f);
        }
    }
    for (e = U32(list, 8); e != 0; e = U32(e, 8)) {
        if (!((U16(e, 2) & 0x100) != 0 && (U16(e, 0) & 0x800) == 0) && (U16(e, 2) & 0xf0) != 0x50)
            continue;
        if (((W10_3FilterFn)(uintptr_t)G32(0x1014cb40))((void *)(uintptr_t)e) != 0)
            continue;
        other = 0;
        r = U32(e, 0x28);
        if (r != 0 && U32(r, 0) != 0)
            other = U32(U32(r, 0), 0x8c);
        if (G32(0x1015e550) != 0) {
            /* the original's three tests, with the same calls */
            if (Str_CompareNoCase((const char *)(uintptr_t)(e + 0xa4), W10_3NAME) != 0 && other == 0)
                continue;
            if (Str_CompareNoCase((const char *)(uintptr_t)(e + 0xa4), W10_3NAME) != 0 && other != 0 &&
                Str_CompareNoCase((const char *)(uintptr_t)(other + 0xa4), W10_3NAME) != 0)
                continue;
            x = *(float *)(BYTES(e) + 0x7c);
            y = *(float *)(BYTES(e) + 0x80);
            z = *(float *)(BYTES(e) + 0x84);
            rad = *(float *)(BYTES(e) + 0x88);
        } else {
            x = *(float *)(BYTES(e) + 0x44);
            y = *(float *)(BYTES(e) + 0x48);
            z = *(float *)(BYTES(e) + 0x4c);
            rad = *(float *)(BYTES(e) + 0x50);
        }
        colour = Str_CompareNoCase((const char *)(uintptr_t)(e + 0xa4), W10_3NAME) == 0 ? 0xf : 7;
        if (Sim_100d2669(&x, &y, &z) == 0)
            continue;
        cam = (const float *)(uintptr_t)G32(0x1014cae0);
        /* x87 at 53-bit precision: each step rounds to double, the fstp to float, as here */
        rad = (float)((double)cam[0x25] * (double)rad / (double)z);
        w = Render_Ftol((double)rad * (double)cam[0x11]);
        SIM_FN(W10_3BoxFn, 0x100eb6e8)(GBYTES(0x109e3e00), Render_Ftol(x), Render_Ftol(y), Render_Ftol(rad), w, colour);
        if (G32(0x1015e554) == 0)
            G32(0x1015e554) = (uint32_t)(uintptr_t)SIM_FN(W10_3LoadFn, 0x100ba7d5)(
                (void *)(uintptr_t)G32(0x109b9900), (void *)(uintptr_t)G32(0x1014cec4), 0);
        if (G32(0x1015e554) == 0)
            continue;
        if (Render_Ftol((double)y - (double)rad) <= 0)
            continue;
        SIM_FN(W10_3TextFn, 0x100ecea6)(GBYTES(0x109e3e00), Render_Ftol(x), Render_Ftol((double)y - (double)rad),
                                        (void *)(uintptr_t)G32(0x1015e554), (const char *)(uintptr_t)(e + 0xa4),
                                        SIM_PTR(void, 0x109b9920));
    }
}

static uint32_t W10_3Bits(float f)
{
    W10_3Word w;

    w.f = f;
    return w.u;
}

// FUNCTION: HGSIM 0x10031447
void *__cdecl Sim_10031447(void *box, uint32_t q, void *mesh)
{
    const float *b = (const float *)box;
    float x0, x1, z0, z1;
    W10_3Word yMin, yMax, fMin, fMax;
    uint32_t faces[100];     /* the original's 100 pointers at ebp-0x198 */
    int32_t count = 0, i, k;
    uint8_t *node, *child;

    /* x87 at 53-bit precision: each step rounds to double, the fstp to float */
    if (q & 1) {
        x0 = (float)(((double)b[1] + (double)b[0]) / (double)*SIM_PTR(float, 0x101112f0) +
                     (double)*SIM_PTR(float, 0x101112f4));
        x1 = b[1];
    } else {
        x0 = b[0];
        x1 = (float)(((double)b[1] + (double)b[0]) / (double)*SIM_PTR(float, 0x101112f0));
    }
    if (q & 2) {
        z0 = (float)(((double)b[5] + (double)b[4]) / (double)*SIM_PTR(float, 0x101112f0) +
                     (double)*SIM_PTR(float, 0x101112f4));
        z1 = b[5];
    } else {
        z0 = b[4];
        z1 = (float)(((double)b[5] + (double)b[4]) / (double)*SIM_PTR(float, 0x101112f0));
    }
    if (x0 >= x1 || z0 >= z1)
        return 0;
    yMin.u = 0x7f7fc99eu;
    yMax.u = 0xff7fc99eu;
    for (i = 0; i < S16(mesh, 6); i++) {
        uint8_t *face = BYTES(mesh) + U32(mesh, 8) + (uint32_t)i * 0x28u;

        if (Sim_1003196f(face, mesh, x0, x1, z0, z1, &fMin.u, &fMax.u) == 0)
            continue;
        count++;
        if (count > 100) {
            yMin.u = 0x7f7fc99eu;
            yMax.u = 0xff7fc99eu;
            node = (uint8_t *)Sim_1003184b(W10_3Bits(x0), W10_3Bits(x1), yMin.u, yMax.u, W10_3Bits(z0),
                                           W10_3Bits(z1), 0);
            if (node == 0)
                return 0;
            for (k = 0; k < 4; k++) {
                U32(node, 0x1c + k * 4) = (uint32_t)(uintptr_t)Sim_10031447(node, (uint32_t)k, mesh);
                child = BYTES(U32(node, 0x1c + k * 4));
                if (child == 0)
                    continue;
                if (*(float *)(child + 0xc) > yMax.f)
                    yMax.u = U32(child, 0xc);
                if (!(*(float *)(child + 8) >= yMin.f))
                    yMin.u = U32(child, 8);
            }
            U32(node, 0xc) = yMax.u;
            U32(node, 8) = yMin.u;
            /* fcomp: equal or unordered */
            if (!(*(float *)(node + 8) < *SIM_PTR(float, 0x101112f8) ||
                  *(float *)(node + 8) > *SIM_PTR(float, 0x101112f8))) {
                U32(node, 0xc) = 0;
                U32(node, 8) = 0;
            }
            return node;
        }
        if (fMax.f > yMax.f)
            yMax.u = fMax.u;
        if (!(fMin.f >= yMin.f))
            yMin.u = fMin.u;
        faces[count - 1] = (uint32_t)(uintptr_t)face;
    }
    if (count == 0) {
        yMax.u = 0;
        yMin.u = 0;
    }
    node = (uint8_t *)Sim_1003184b(W10_3Bits(x0), W10_3Bits(x1), yMin.u, yMax.u, W10_3Bits(z0), W10_3Bits(z1),
                                   count);
    if (node == 0)
        return 0;
    if (count > 0)
        memcpy(node + 0x2c, faces, (uint32_t)count * 4u);
    return node;
}

/* The original's reject test of a box against the segment's bounds (fcomp: NaN rejects on the "not below" side). */
static int W10_3Outside(const float *b, float minx, float maxx, float miny, float maxy, float minz, float maxz)
{
    return minx > b[1] || !(maxx >= b[0]) || miny > b[3] || !(maxy >= b[2]) || minz > b[5] || !(maxz >= b[4]);
}

static int32_t W10_3Cell(float v, float origin)
{
    /* x87 at 53-bit precision: ((v - origin) + 28000) / 8000, truncated */
    return Render_Ftol((((double)v - (double)origin) + (double)*SIM_PTR(float, 0x101112e0)) /
                       (double)*SIM_PTR(float, 0x101112e4));
}

// FUNCTION: HGSIM 0x10030c87
int __cdecl Sim_10030c87(const float *grid, uint8_t *points, void *seg)
{
    const float *s = (const float *)seg;
    uint8_t tmp[0x38], best[0x38];     /* the original's two segments at ebp-0x40 and ebp-0x7c */
    W10_3Word bestLen, len;
    float minx, maxx, miny, maxy, minz, maxz;
    int32_t i0, i1, k0, k1, i, k;
    uint32_t node, face;

    bestLen.u = 0x7f7fc99eu;
    if (grid == 0 || points == 0 || seg == 0)
        return 0;
    minx = maxx = s[0];
    miny = maxy = s[1];
    minz = maxz = s[2];
    if (!(s[3] >= minx))
        minx = s[3];
    if (s[3] > maxx)
        maxx = s[3];
    if (!(s[5] >= minz))
        minz = s[5];
    if (s[5] > maxz)
        maxz = s[5];
    if (!(s[4] >= miny))
        miny = s[4];
    if (s[4] > maxy)
        maxy = s[4];
    if (W10_3Outside(grid, minx, maxx, miny, maxy, minz, maxz))
        return 0;
    i0 = W10_3Cell(minx, grid[6]);
    i1 = W10_3Cell(maxx, grid[6]);
    k0 = W10_3Cell(minz, grid[8]);
    k1 = W10_3Cell(maxz, grid[8]);
    if (i0 > 7 || i1 < 0 || k0 > 7 || k1 < 0)
        return 0;
    for (i = i0; i <= i1; i++) {
        if (i < 0 || i >= 7)
            continue;
        for (k = k0; k <= k1; k++) {
            const uint8_t *cell;

            if (k < 0 || k >= 7)
                continue;
            cell = (const uint8_t *)grid + 0x24 + i * 0xe0 + k * 0x20;
            if (W10_3Outside((const float *)cell, minx, maxx, miny, maxy, minz, maxz) || U32(cell, 0x1c) == 0 ||
                U32(cell, 0x18) == 0)
                continue;
            for (node = U32(cell, 0x1c); node != 0; node = U32(node, 0)) {
                face = U32(node, 4);
                Sim_1009cfbd(tmp, seg);
                if (face == 0 || Sim_1002e790((void *)(uintptr_t)face, points + 0x18, tmp) == 0)
                    continue;
                len.f = Sim_1009cbde(tmp);
                if (!(len.f >= bestLen.f)) {
                    bestLen.u = len.u;
                    Sim_1009cfbd(best, tmp);
                }
            }
        }
    }
    if (bestLen.f >= *SIM_PTR(float, 0x101112ec))
        return 0;
    Sim_1009cfbd(seg, best);
    return 1;
}


typedef void *(__stdcall *W10_3HeapAlloc2Fn)(void *heap, uint32_t flags, uint32_t bytes);

/* the 17 cleared blocks of 0x100 bytes, in the original's order (the 14th clears 0x1015ecb8 again, not its own);
   1.2 (0x10031389) also allocates the "DI" message buffer (its [0x10131504], g_net10DiBuf) after the third, the {0, 0}
   entry here, and its "AK" buffer after the fifth (ours is the static g_w34AkBuf, round 33) */
static const uint32_t W10_3BLOCKS[18][2] = {
    {0x1015ec88, 0x1015ec88}, {0x1015ec90, 0x1015ec90}, {0x1015ecb0, 0x1015ecb0}, {0, 0}, {0x1015ec98, 0x1015ec98},
    {0x1015d084, 0x1015d084}, {0x1015ec9c, 0x1015ec9c}, {0x1015eca0, 0x1015eca0}, {0x1015eca4, 0x1015eca4},
    {0x1015ecbc, 0x1015ecbc}, {0x1015eca8, 0x1015eca8}, {0x1015ecac, 0x1015ecac}, {0x1015ecb4, 0x1015ecb4},
    {0x1015ecb8, 0x1015ecb8}, {0x1015ecc0, 0x1015ecb8}, {0x1015ecc4, 0x1015ecc4}, {0x1015ecc8, 0x1015ecc8},
    {0x1015ec94, 0x1015ec94},
};

// FUNCTION: HGSIM 0x1003dc19
void __cdecl Sim_1003dc19(uint32_t game)
{
    uint8_t *root;
    int seen = 0, i, k;
    uint16_t id;

    if (game != 0) {
        G32(0x1015d004) = game;
        G32(0x1015ec84) = 1;
    }
    if (G32(0x1015ec58) == 0) {
        G32(0x1017b94c) |= 1;
        return;
    }
    for (i = 0; i < NET10_PLAYERS; i++) {     /* 1.2: 10 players, its own tables (1.0: 8, 0x1015db00 ...) */
        g_net10Heard[i] = 0;
        g_net10Mark[i] = 0;
        g_net10Ready[i] = 0;
        g_net10Left[i] = 0;
    }
    *SIM_PTR(u16a, 0x1016552a) = *SIM_PTR(u16a, 0x1015dc64);
    G32(0x1015d088) = 0;
    G32(0x1015ec8c) = 0x42;
    for (i = 0; i < 18; i++) {
        void *block = SIM_API(W10_3HeapAlloc2Fn, 0x10ac5658)((void *)(uintptr_t)G32(0x1017b8f8), 1, 0x100);

        if (W10_3BLOCKS[i][0] == 0) {
            g_net10DiBuf = (uint8_t *)block;
            if (block == 0)
                return;
            Mem_Fill(block, 0, 0x100);
            continue;
        }
        G32(W10_3BLOCKS[i][0]) = (uint32_t)(uintptr_t)block;
        if (block == 0)
            return;
        Mem_Fill((void *)(uintptr_t)G32(W10_3BLOCKS[i][1]), 0, i == 0 ? G32(0x1015ec8c) : 0x100);
        if (i == 14)
            G32(0x1015d08c) = 0x27;
    }
    G32(0x1015d008) = (uint32_t)(uintptr_t)SIM_API(W10_3HeapAlloc2Fn, 0x10ac5658)(
        (void *)(uintptr_t)G32(0x1017b8f8), 1, 0x100);
    if (G32(0x1015d008) == 0)
        return;
    for (i = 0; i < NET10_PLAYERS; i++) {
        g_net10Db20[i] = 0;
        g_net10SnStamp[i] = 0;
#ifdef NET10_MUTATE /* the checker's negative check only: 1.0's 0x11 bytes from +5 */
        for (k = 0; k < 0x11; k++)
            g_net10Row[i][5 + k] = 0;
#else
        for (k = 0; k < 0x16; k++)              /* 1.2: the whole row */
            g_net10Row[i][k] = 0;
#endif
        for (k = 0; k < 0xd1; k++)              /* 1.2: the last "DI" message sent per player */
            g_net10DiSent[i][k] = 0;
    }
    root = BYTES(G32(0x1015d004));
    G32(0x1017b8c8) = Sim_1003db56((int16_t)U16(root, 0x10));
    *SIM_PTR(u16a, 0x1015ecd8) = 0xfa01;
    for (i = 0; i < S32(G32(0x1015d004), 0x12); i++) {
        id = U16(U32(G32(0x1015d004), 0x16), (uint32_t)i * 2u);
        if (id == 0xfa01)
            continue;
        if (seen) {
            if (id < *SIM_PTR(u16a, 0x1015ecd8))
                *SIM_PTR(u16a, 0x1015ecd8) = id;
        } else {
            seen = 1;
            *SIM_PTR(u16a, 0x1015ecd8) = id;
        }
    }
    *SIM_PTR(u16a, 0x1013acac) = U16(G32(0x1015d004), 0x10);
    if (*SIM_PTR(u16a, 0x1015ecd8) == *SIM_PTR(u16a, 0x1013acac)) {
        G32(0x1015ecd0) = 1;
        G32(0x1015dc84) = 0;
    } else {
        G32(0x1015ecd0) = 2;
        G32(0x1015dc84) = 2;
    }
}