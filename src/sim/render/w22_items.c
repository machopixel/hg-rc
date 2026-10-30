#include <string.h>

#include "w22.h"
#include "w2n.h"
#include "w3p.h"
#include "../unsorted_100a/w6g.h"
#include "../unsorted_100d/w8z.h"
#include "../util/access.h"

typedef float HG_MAY_ALIAS w22f;

#define W22_FL(p, off) (*(w22f *)(BYTES(p) + (off)))
#define W22_CALL_100585c4 SIM_FN(int(__cdecl *)(int), 0x100585c4)
#define W22_CALL_100ac9ab SIM_FN(void(__cdecl *)(void *), 0x100ac9ab)

// FUNCTION: HGSIM 0x100ad96e
void __cdecl Sim_100ad96e(int32_t idx)
{
    uint8_t *item, *obj, *model;
    float x, y, z;

    if (idx < 0 || idx >= GS32(0x1017d1c8))
        return;
    item = GBYTES(0x101c57e0) + (uint32_t)idx * 0x34u;
    switch (U32(item, 4)) {
    case 1:
        if (G32(0x1014d1d8) == 0)
            return;
        obj = BYTES(U32(item, 8));
        if (U32(obj, 4) == G32(0x1017b8c8) && G32(0x1015ef0c) != 0)
            return;
        model = BYTES(U32(obj, 0x48));
        if (U32(obj, 0) == 1 || U32(obj, 0) == 9) {
            x = W22_FL(obj, 0x58);
            y = W22_FL(obj, 0x5c);
            z = W22_FL(obj, 0x60);
            Sim_100d2669(&x, &y, &z);
            if ((z > *SIM_PTR(w22f, 0x1012bab0) || U32(obj, 0) == 9) && U32(model, 0x8c) != 0)
                Sim_100ad5b7(W22_FL(item, 0x14), W22_FL(item, 0x18), W22_FL(item, 0x1c),
                             W22_FL(U32(model, 0x8c), 0x50));
            else
                Sim_100ad87d(item, model);
        } else {
            Sim_100ad87d(item, model);
        }
        return;
    case 2:
    case 6:
        if (W22_CALL_100585c4(-1) != 0)
            W22_CALL_100ac9ab(item);
        return;
    default:
        return;
    }
}

#define W22_CALL_100ac738 SIM_FN(void(__cdecl *)(void *, uint32_t), 0x100ac738)

/* one 0x58-byte clip vertex: world x / y / z at +0xc / +0x10 / +0x14, u +0x18, v +0x1c 0, +0x2c (cached vertex) 0 */
static void W22_ClipVertex(uint8_t *v, int32_t x, int32_t y, int32_t z, uint32_t u)
{
    W22_FL(v, 0xc) = (float)x;
    W22_FL(v, 0x14) = (float)z;
    W22_FL(v, 0x10) = (float)y;
    U32(v, 0x18) = u;
    U32(v, 0x1c) = 0;
    U32(v, 0x2c) = 0;
}

// FUNCTION: HGSIM 0x100ac9ab
void __cdecl Sim_100ac9ab(void *item)
{
    /* +0..+8 and +0x31..+0x57 are never written by the original (DECISIONS.md round 22: nothing reads them) */
    uint8_t quad[4 * 0x58];
    int32_t x = Render_Ftol(W22_FL(item, 0x14)), y = Render_Ftol(W22_FL(item, 0x18));
    int32_t z = Render_Ftol(W22_FL(item, 0x1c)), s = Render_Ftol(W22_FL(item, 0x2c)), h = s + s / 2;
    static const int8_t dir[4][3][2] = {
        {{1, -1}, {2, 0}, {1, 1}}, {{-1, -1}, {0, -2}, {1, -1}}, {{-1, 1}, {-2, 0}, {-1, -1}}, {{1, 1}, {0, 2}, {-1, 1}},
    };
    int32_t q, k, dx, dz;

    memset(quad, 0, sizeof(quad));
    G32(0x101c9ea8) = 0;
    for (q = 0; q < 4; q++) {
        W22_ClipVertex(quad, x, y, z, 0x41c00000u);
        for (k = 0; k < 3; k++) {
            dx = dir[q][k][0];
            dz = dir[q][k][1];
            /* +-1 is the size, +-2 the size and a half */
            W22_ClipVertex(quad + (k + 1) * 0x58, x + (dx == 2 ? h : dx == -2 ? -h : dx * s), y,
                           z + (dz == 2 ? h : dz == -2 ? -h : dz * s), 0x41800000u);
        }
        W22_CALL_100ac738(quad, 4);
    }
}

#define W22_CALL_100a9cfb SIM_FN(void(__cdecl *)(uint32_t, void *, void *, void *), 0x100a9cfb)
#define W22_CALL_100e84f0 SIM_FN(void(__cdecl *)(void *, int32_t, int32_t, uint32_t), 0x100e84f0)
typedef int(__cdecl *W22DriverFn)(int32_t count, void *verts, uint32_t flags);

/* x87: an integer plus a float, rounded once to float */
static float W22_AddI(int32_t i, float f)
{
    return (float)((double)i + (double)f);
}

// FUNCTION: HGSIM 0x100945b8
int __cdecl Sim_100945b8(void *obj)
{
    /* the four plugin vertices: z (+8) and the fields 0x1009ef50 refills are never written by the original, which
       hands over stack leftovers (DECISIONS.md round 22): 0 here, z included */
    uint8_t v[4 * 0x24];
    float base[3], scale[3], x, y, z, s;
    int32_t i, size, a, b;
    uint32_t colour = 0xf0;

    if (obj == 0)
        return 0;
    memset(v, 0, sizeof(v));
    W22_CALL_100a9cfb(U32(obj, 0x552), &base[0], &base[1], &base[2]);
    scale[0] = (float)((double)W22_FL(obj, 0x56e) / (double)*SIM_PTR(w22f, 0x1012b958));
    scale[1] = (float)((double)W22_FL(obj, 0x572) / (double)*SIM_PTR(w22f, 0x1012b958));
    scale[2] = (float)((double)W22_FL(obj, 0x576) / (double)*SIM_PTR(w22f, 0x1012b958));
    (void)scale;
    for (i = 0; i < 0x10; i++) {
        x = (float)((double)W22_FL(obj, 0x12 + i * 0x2a) + (double)base[0]);
        y = (float)((double)W22_FL(obj, 0x16 + i * 0x2a) + (double)base[1]);
        z = (float)((double)W22_FL(obj, 0x1a + i * 0x2a) + (double)base[2]);
        if (Sim_100d2669(&x, &y, &z) == 0)
            continue;
        size = Render_Ftol((double)(int32_t)(U32(obj, 0x56a) * 0x7d0u) / (double)z);
        if (G32(0x1017b940) == 0) {
            if (size < 1) {
                W22_CALL_100e84f0(GBYTES(0x109e3e00), Render_Ftol(x), Render_Ftol(y), colour);
                continue;
            }
            for (a = 0; a < size; a += 2) {
                for (b = 0; b < size; b += 2) {
                    float px = W22_AddI(a, x), py = W22_AddI(b, y);

                    W22_CALL_100e84f0(GBYTES(0x109e3e00), Render_Ftol(px), Render_Ftol(py), colour);
                    px = (float)((double)px + (double)*SIM_PTR(w22f, 0x1012b970));
                    py = (float)((double)py + (double)*SIM_PTR(w22f, 0x1012b970));
                    W22_CALL_100e84f0(GBYTES(0x109e3e00), Render_Ftol(px), Render_Ftol(py), colour);
                }
            }
            continue;
        }
        if (size < 1)
            size = 1;
        {
            int32_t c = GS32(0x10185760 + colour * 4);

            v[0x14] = (uint8_t)(c >> 16);
            v[0x15] = (uint8_t)(c >> 8);
            v[0x16] = (uint8_t)c;
        }
        s = (float)((double)*SIM_PTR(w22f, 0x1012b970) / (double)z);
        /* v3 (x, y), v2 (x + size, y), v1 (x + size, y + size), v0 (x, y + size) */
        W22_FL(v + 0x6c, 0) = x;
        W22_FL(v + 0x6c, 4) = y;
        W22_FL(v + 0x48, 0) = W22_AddI(size, x);
        W22_FL(v + 0x48, 4) = y;
        W22_FL(v + 0x24, 0) = W22_AddI(size, x);
        W22_FL(v + 0x24, 4) = W22_AddI(size, y);
        W22_FL(v, 0) = x;
        W22_FL(v, 4) = W22_AddI(size, y);
        for (a = 0; a < 4; a++) {
            U32(v + a * 0x24, 0x10) = 0xf;
            U32(v + a * 0x24, 0x1c) = 0;
            U32(v + a * 0x24, 0x20) = 0x3f800000u;
            W22_FL(v + a * 0x24, 0xc) = s;
        }
        ((W22DriverFn)(uintptr_t)G32(0x1014cb4c))(4, v, colour);
    }
    return 1;
}

/* the quad of a spark: v3 (x, y), v2 (x + size, y), v1 (x + size, y + size), v0 (x, y + size); light 0xf, u 0,
   v 1.0, the given 1/z; z (+8) and the colour / +0x18 fields are left as they are (0 at entry) */
static void W22_SparkQuad(uint8_t *v, float x, float y, int32_t size, float rhw)
{
    int32_t a;

    W22_FL(v + 0x6c, 0) = x;
    W22_FL(v + 0x6c, 4) = y;
    W22_FL(v + 0x48, 0) = W22_AddI(size, x);
    W22_FL(v + 0x48, 4) = y;
    W22_FL(v + 0x24, 0) = W22_AddI(size, x);
    W22_FL(v + 0x24, 4) = W22_AddI(size, y);
    W22_FL(v, 0) = x;
    W22_FL(v, 4) = W22_AddI(size, y);
    for (a = 0; a < 4; a++) {
        U32(v + a * 0x24, 0x10) = 0xf;
        U32(v + a * 0x24, 0x1c) = 0;
        U32(v + a * 0x24, 0x20) = 0x3f800000u;
        W22_FL(v + a * 0x24, 0xc) = rhw;
    }
}

// FUNCTION: HGSIM 0x1009353e
int __cdecl Sim_1009353e(void *obj)
{
    /* z (+8) of the four plugin vertices: 0 (the original never writes it; DECISIONS.md round 22) */
    uint8_t v[4 * 0x24];
    float base[3], zero, x, y, z;
    int32_t i, size;
    uint32_t colour;

    if (obj == 0)
        return 0;
    memset(v, 0, sizeof(v));
    W22_CALL_100a9cfb(U32(obj, 0x552), &base[0], &base[1], &base[2]);
    colour = U32(obj, 0x566);
    zero = 0;
    z = base[2];
    Sim_100d2669(&zero, &zero, &z);     /* the original projects a point with x and y in one local; unused */
    for (i = 0; i < 0x10; i++) {
        uint8_t *p = BYTES(obj) + i * 0x2a;

        W22_FL(p, 0x12) = (float)((double)W22_FL(p, 0x12) + (double)W22_FL(p, 0x1e));
        W22_FL(p, 0x16) = (float)((double)W22_FL(p, 0x16) + (double)W22_FL(p, 0x22));
        W22_FL(p, 0x1a) = (float)((double)W22_FL(p, 0x1a) + (double)W22_FL(p, 0x26));
        x = (float)((double)base[0] + (double)W22_FL(p, 0x12));
        y = (float)((double)base[1] + (double)W22_FL(p, 0x16));
        z = (float)((double)base[2] + (double)W22_FL(p, 0x1a));
        if (Sim_100d2669(&x, &y, &z) == 0)
            continue;
        size = Render_Ftol((double)(int32_t)(U32(obj, 0x56a) * 0x7d0u) / (double)z);
        if (size < 1) {
            if (G32(0x1017b940) == 0) {
                W22_CALL_100e84f0(GBYTES(0x109e3e00), Render_Ftol(x), Render_Ftol(y), colour);
                continue;
            }
            size = 1;
        }
        W22_SparkQuad(v, x, y, size, 1.0f);
        ((W22DriverFn)(uintptr_t)G32(0x1014cb4c))(4, v, colour);
    }
    return 1;
}

// FUNCTION: HGSIM 0x1009596a
int __cdecl Sim_1009596a(void *obj)
{
    /* z (+8) of the four plugin vertices: 0 (the original never writes it; DECISIONS.md round 22) */
    uint8_t v[4 * 0x24];
    float base[3], scale[3], zero, x, y, z, x2, y2, z2;
    int32_t i, near;
    uint32_t colour;

    if (obj == 0)
        return 0;
    memset(v, 0, sizeof(v));
    W22_CALL_100a9cfb(U32(obj, 0x552), &base[0], &base[1], &base[2]);
    scale[0] = (float)((double)W22_FL(obj, 0x56e) / (double)*SIM_PTR(w22f, 0x1012b958));
    scale[1] = (float)((double)W22_FL(obj, 0x572) / (double)*SIM_PTR(w22f, 0x1012b958));
    scale[2] = (float)((double)W22_FL(obj, 0x576) / (double)*SIM_PTR(w22f, 0x1012b958));
    colour = U32(obj, 0x566);
    zero = 0;
    z = base[2];
    Sim_100d2669(&zero, &zero, &z);     /* the depth of the base point (x and y in one local, as in the original) */
    near = !(z > *SIM_PTR(w22f, 0x1012b980));
    for (i = 0; i < 0x10; i++) {
        uint8_t *p = BYTES(obj) + i * 0x2a;

        x = (float)((double)W22_FL(p, 0x12) + (double)base[0]);
        y = (float)((double)W22_FL(p, 0x16) + (double)base[1]);
        z = (float)((double)W22_FL(p, 0x1a) + (double)base[2]);
        if (Sim_100d2669(&x, &y, &z) == 0)
            continue;
        if (near) {         /* a second, scaled point is projected too; its result is not used */
            x2 = (float)((double)W22_FL(p, 0x12) + (double)base[0] + (double)scale[0]);
            y2 = (float)((double)W22_FL(p, 0x16) + (double)base[1] + (double)scale[1]);
            z2 = (float)((double)W22_FL(p, 0x1a) + (double)base[2] + (double)scale[2]);
            Sim_100d2669(&x2, &y2, &z2);
        }
        W22_SparkQuad(v, x, y, 1, 1.0f);
        ((W22DriverFn)(uintptr_t)G32(0x1014cb4c))(4, v, colour);
    }
    return 1;
}

/* one beam quad: v3 = B + (sx, sy) * near, v2 = A + (sx, sy) * near, v1 = A + (sx, sy) * far, v0 = B + (sx, sy) *
   far, where near / far are offsets (0 means the point itself); light 0xff, 1/z 1.0, u 0, v 1.0 */
static void W22_BeamQuad(uint8_t *v, const float *a, const float *b, int sx, int sy, const w22f *nearOff,
                         const w22f *farOff)
{
    int k;

    W22_FL(v + 0x6c, 0) = sx && nearOff ? (float)((double)b[0] + (double)*nearOff) : b[0];
    W22_FL(v + 0x6c, 4) = sy && nearOff ? (float)((double)b[1] + (double)*nearOff) : b[1];
    W22_FL(v + 0x48, 0) = sx && nearOff ? (float)((double)a[0] + (double)*nearOff) : a[0];
    W22_FL(v + 0x48, 4) = sy && nearOff ? (float)((double)a[1] + (double)*nearOff) : a[1];
    W22_FL(v + 0x24, 0) = sx && farOff ? (float)((double)a[0] + (double)*farOff) : a[0];
    W22_FL(v + 0x24, 4) = sy && farOff ? (float)((double)a[1] + (double)*farOff) : a[1];
    W22_FL(v, 0) = sx && farOff ? (float)((double)b[0] + (double)*farOff) : b[0];
    W22_FL(v, 4) = sy && farOff ? (float)((double)b[1] + (double)*farOff) : b[1];
    for (k = 0; k < 4; k++) {
        U32(v + k * 0x24, 0xc) = 0x3f800000u;
        U32(v + k * 0x24, 0x10) = 0xff;
        U32(v + k * 0x24, 0x1c) = 0;
        U32(v + k * 0x24, 0x20) = 0x3f800000u;
    }
}

static int W22_Beam(void *obj)
{
    /* z (+8) of the four plugin vertices: 0 (the original never writes it; DECISIONS.md round 22) */
    uint8_t v[4 * 0x24];
    float a[3], b[3], dx, dy;
    int sx, sy;

    if (obj == 0)
        return 0;
    memset(v, 0, sizeof(v));
    a[0] = W22_FL(obj, 0x12);
    a[1] = W22_FL(obj, 0x16);
    a[2] = W22_FL(obj, 0x1a);
    b[0] = W22_FL(obj, 0x1e);
    b[1] = W22_FL(obj, 0x22);
    b[2] = W22_FL(obj, 0x26);
    Sim_100d2669(&a[0], &a[1], &a[2]);
    Sim_100d2669(&b[0], &b[1], &b[2]);
    dx = (float)((double)a[0] - (double)b[0]);
    dy = (float)((double)a[1] - (double)b[1]);
    if (!(dx >= *SIM_PTR(w22f, 0x1012b944)))      /* test ah, 1: less, or unordered */
        dx = -dx;
    if (!(dy >= *SIM_PTR(w22f, 0x1012b944)))
        dy = -dy;
    if (dx > dy) {                  /* a beam wider than high: thickened in y */
        sx = 0;
        sy = 1;
    } else if (!(dx >= dy)) {       /* higher than wide: in x */
        sx = 1;
        sy = 0;
    } else {                        /* diagonal: both */
        sx = 1;
        sy = 1;
    }
    W22_BeamQuad(v, a, b, sx, sy, SIM_PTR(w22f, 0x1012b948), 0);
    ((W22DriverFn)(uintptr_t)G32(0x1014cb4c))(4, v, (uint32_t)(S32(obj, 0x566) >> 1));
    W22_BeamQuad(v, a, b, sx, sy, SIM_PTR(w22f, 0x1012b94c), SIM_PTR(w22f, 0x1012b950));
    ((W22DriverFn)(uintptr_t)G32(0x1014cb4c))(4, v, U32(obj, 0x566));
    return 1;
}

// FUNCTION: HGSIM 0x100918e6
int __cdecl Sim_100918e6(void *obj)
{
    return W22_Beam(obj);
}

// FUNCTION: HGSIM 0x1009221d
int __cdecl Sim_1009221d(void *obj)
{
    return W22_Beam(obj);
}
