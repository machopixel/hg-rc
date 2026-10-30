#include <string.h>

#include "w22.h"
#include "w2n.h"
#include "../util/access.h"
#include "../unsorted_1008/w8w.h"
#include "../util/util.h"

typedef float HG_MAY_ALIAS w22pf;
typedef int(__cdecl *W22pDriverFn)(int32_t count, void *verts, uint32_t flags);

#define W22P_FL(p, off) (*(w22pf *)(BYTES(p) + (off)))
#define W22P_VERTS 0x101c9ec0u
#define W22P_DRIVER(n, flags) \
    ((W22pDriverFn)(uintptr_t)G32(0x1014cb4c))((n), GBYTES(W22P_VERTS), (uint32_t)(flags))
/* the graphics library, still original (plain cdecl) */
#define W22P_CALL_100e84f0 SIM_FN(void(__cdecl *)(void *, int32_t, int32_t, uint32_t), 0x100e84f0)
#define W22P_CALL_100ef727 \
    SIM_FN(void(__cdecl *)(void *, int32_t, int32_t, int32_t, int32_t, uint32_t, uint32_t), 0x100ef727)
#define W22P_CALL_100e86a1 \
    SIM_FN(void(__cdecl *)(void *, int32_t, int32_t, int32_t, int32_t, uint32_t, uint32_t), 0x100e86a1)
#define W22P_CALL_1009fada SIM_FN(void(__cdecl *)(int32_t, void *, uint32_t), 0x1009fada)

// FUNCTION: HGSIM 0x100a539e
void __cdecl Sim_100a539e(void *rec)
{
    int32_t count = S16(rec, 0), n;
    uint32_t *src = (uint32_t *)(BYTES(rec) + 0x10);
    uint8_t *out = GBYTES(W22P_VERTS), *v = GBYTES(W22P_VERTS), *p;
    uint32_t flags = U16(rec, 2), kind;

    for (n = count; n != 0; n--) {
        p = BYTES(*src++);
        W22P_FL(out, 0) = (float)S32(p, 0x18);
        W22P_FL(out, 4) = (float)S32(p, 0x1c);
        U32(out, 8) = U32(p, 8);
        U32(out, 0x1c) = U32(p, 0x20);
        U32(out, 0x20) = U32(p, 0x24);
        U32(out, 0x10) = U32(p, 0x3c);
#ifdef W22P_MUTATE /* the checker's negative check only */
        U32(out, 0x18) = (uint32_t)U8(p, 0x2b) << 16;
#else
        U32(out, 0x18) = (uint32_t)U8(p, 0x2b) << 24;
#endif
        out += 0x24;
    }
    if (count == 1 && G32(0x1014cb00) != 0) {
        if (flags != 0x1000)
            W22P_CALL_100e84f0(GBYTES(0x109e3e00), Render_Ftol(W22P_FL(v, 0)), Render_Ftol(W22P_FL(v, 4)), flags);
        return;
    }
    if (count == 2 && G32(0x1014cafc) != 0) {
        if (G32(0x1017b940) != 0)
            W22P_CALL_100ef727(GBYTES(0x109e3e00), Render_Ftol(W22P_FL(v, 0)), Render_Ftol(W22P_FL(v, 4)),
                               Render_Ftol(W22P_FL(v, 0x24)), Render_Ftol(W22P_FL(v, 0x28)), 0,
                               U16(GBYTES(0x101659e0), flags * 2));
        else
            W22P_CALL_100e86a1(GBYTES(0x109e3e00), Render_Ftol(W22P_FL(v, 0)), Render_Ftol(W22P_FL(v, 4)),
                               Render_Ftol(W22P_FL(v, 0x24)), Render_Ftol(W22P_FL(v, 0x28)), 0, flags);
        return;
    }
    switch (G32(0x1014cb1c)) {
    case 0:
        W22P_DRIVER(count, flags);
        if ((U32(rec, 0xc) & 1) != 0)
            W22P_CALL_1009fada(count, GBYTES(W22P_VERTS), flags);
        return;
    case 3:
        W22P_DRIVER(count, flags);
        return;
    case 1:
        kind = flags & 0xf000;
        switch (kind) {
        case 0x2000: case 0x3000: case 0x5000: case 0x6000: case 0x7000: case 0x8000:
        case 0xb000: case 0xc000: case 0xd000: case 0xe000:
            W22P_DRIVER(count, flags);
            return;
        case 0x9000:
            return;
        default:
            W22P_DRIVER(count, 0xf0);
            W22P_DRIVER(count, flags | 0xa000);
            return;
        }
    case 4:
        if ((flags & 0xff) != 0x10)
            W22P_DRIVER(count, (flags & 0xff) | 0xa000);
        return;
    default:
        W22P_DRIVER(count, flags | 0xa000);
        return;
    }
}

#define W22P_CALL_1009c929 SIM_FN(int32_t(__cdecl *)(int32_t), 0x1009c929)
#define W22P_CALL_100a2b8c SIM_FN(void(__cdecl *)(int32_t, int32_t, uint32_t), 0x100a2b8c)
typedef int(__cdecl *W22pRenderFn)(int mode, void *surface, void *verts, int count, void *tex);

/* x87 fadd of two floats then fstp to float: the sum is rounded once */
static float W22p_Add(int32_t r, float f)
{
    return (float)((double)r + (double)f);
}

// FUNCTION: HGSIM 0x1009fada
void __cdecl Sim_1009fada(int32_t count, void *verts)
{
    uint8_t spark[8 * 0x24];
    uint32_t tex[2];
    float xmin = 3.4028235e38f, xmax = -3.4028235e38f, ymin = 3.4028235e38f, ymax = -3.4028235e38f;
    int32_t cx, cy, i, j;
    uint8_t *v = BYTES(verts), *base, *out;
    uint32_t colour;

    memset(spark, 0, sizeof(spark));
    for (i = 0; i < count; i++, v += 0x24) {
        if (!(W22P_FL(v, 0) >= xmin))       /* fcomp / test ah, 1: less, or unordered */
            xmin = W22P_FL(v, 0);
        if (W22P_FL(v, 0) > xmax)
            xmax = W22P_FL(v, 0);
        if (!(W22P_FL(v, 4) >= ymin))
            ymin = W22P_FL(v, 4);
        if (W22P_FL(v, 4) > ymax)
            ymax = W22P_FL(v, 4);
    }
    cx = Render_Ftol((double)xmax + (double)xmin) >> 1;
    cy = Render_Ftol((double)ymax + (double)ymin) >> 1;
    out = spark;
    for (i = 0; i < 2; i++) {
        base = BYTES(verts) + ((int32_t)(G32(0x1017d110) + (uint32_t)i) % count) * 0x24;
        for (j = 0; j < 4; j++, out += 0x24) {
            if (W22P_CALL_1009c929(100) < 0x32) {
                out[0x14] = 0;
                out[0x15] = 0;
            } else {
                out[0x14] = 0xff;
                out[0x15] = 0xff;
            }
            out[0x16] = 0xff;
            U32(out, 0x10) = 0xff;
            U32(out, 0x18) = 0xff000000u;
            U32(out, 8) = U32(base, 8);
            U32(out, 0xc) = U32(base, 0xc);
            U32(out, 0x1c) = U32(base, 0x1c);
            U32(out, 0x20) = U32(base, 0x20);
            if (j == 0) {
                W22P_FL(out, 0) = W22p_Add(W22P_CALL_1009c929(2), (float)cx);
                W22P_FL(out, 4) = W22p_Add(W22P_CALL_1009c929(2), (float)cy);
            } else {
                W22P_FL(out, 0) = W22p_Add(W22P_CALL_1009c929(5), W22P_FL(base, 0));
                W22P_FL(out, 4) = W22p_Add(W22P_CALL_1009c929(5), W22P_FL(base, 4));
            }
        }
    }
    if (G32(0x1017b940) != 0) {
        tex[0] = 0;
        tex[1] = 0x40;
        ((W22pRenderFn)(uintptr_t)G32(0x109e3d30))(0x10, GBYTES(0x109e3dc0), spark, 8, tex);
    } else {
        G32(0x101cc2bc) = (uint32_t)Render_Ftol(W22P_FL(spark, 0));
        G32(0x101ccc6c) = (uint32_t)Render_Ftol(W22P_FL(spark, 4));
        for (i = 1; i < 8; i++) {
            colour = W22P_CALL_1009c929(100) < 0x32 ? 0x17 : 0xfe;
            W22P_CALL_100a2b8c(Render_Ftol(W22P_FL(spark + i * 0x24, 0)), Render_Ftol(W22P_FL(spark + i * 0x24, 4)),
                               colour);
        }
        colour = W22P_CALL_1009c929(100) < 0x32 ? 0x17 : 0xfe;
        W22P_CALL_100a2b8c(Render_Ftol(W22P_FL(spark, 0)), Render_Ftol(W22P_FL(spark, 4)), colour);
    }
    G32(0x1017d110) += 1;
}

#define W22P_CALL_10090f17 SIM_FN(void(__cdecl *)(uint32_t), 0x10090f17)
#define W22P_CALL_10098f30 SIM_FN(void(__cdecl *)(uint32_t, int32_t, void *, int32_t), 0x10098f30)
#define W22P_CALL_10006744 SIM_FN(int(__cdecl *)(uint32_t, int32_t, void *, int32_t), 0x10006744)
#define W22P_CALL_10006609 \
    SIM_FN(int(__cdecl *)(uint32_t, int32_t, void *, uint32_t, uint32_t, uint32_t, uint32_t), 0x10006609)
#define W22P_CALL_100a2d91 SIM_FN(int32_t(__cdecl *)(int32_t, uint32_t), 0x100a2d91)
#define W22P_CALL_100e4780 SIM_FN(void(__cdecl *)(void *, int32_t, void *), 0x100e4780)
#define W22P_CALL_100e5477 SIM_FN(void(__cdecl *)(void *, int32_t, int32_t, void *), 0x100e5477)

/* the colour bytes +0x14..+0x16 of a plugin vertex from palette entry index, scaled by the three channel factors */
static void W22p_Shade(uint8_t *v, uint32_t index)
{
    int32_t c = GS32(0x10185760 + index * 4);

    v[0x14] = (uint8_t)((((c >> 16) & 0xff) * GS32(0x10143fdc)) >> 8);
    v[0x15] = (uint8_t)((((c >> 8) & 0xff) * GS32(0x10143fe0)) >> 8);
    v[0x16] = (uint8_t)(((c & 0xff) * GS32(0x10143fe4)) >> 8);
}

/* the 0x18-byte software vertices at 0x101ce4a0 with their clip bits (2 / 1: x below / above +-0x2000, 4 / 8: y) */
static uint32_t W22p_Soft(int32_t count, uint8_t *verts, uint32_t flags, int gouraud)
{
    uint32_t bits = 0;
    int32_t *p = (int32_t *)GBYTES(0x101ce4a0), x, y, i;

    for (i = 0; i < count; i++, p += 6) {
        uint8_t *v = verts + i * 0x24;

        p[0] = Render_Ftol(W22P_FL(v, 0));
        x = p[0];
        p[1] = Render_Ftol(W22P_FL(v, 4));
        y = p[1];
        p[2] = (int32_t)((gouraud ? U32(v, 0x10) + (flags & 0xf0) : flags) << 16);
        if (x < -0x2000)
            bits |= 2;
        else if (x > 0x2000)
            bits |= 1;
        if (y < -0x2000)
            bits |= 4;
        else if (y > 0x2000)
            bits |= 8;
    }
    return bits;
}

/* the renderer [0x109e3d30] with a texture record of {0, rec1} (see Sim_1009ef50) */
static void W22p_Render(int mode, void *verts, int32_t count, uint32_t rec1)
{
    uint32_t rec[2];

    rec[0] = 0;
    rec[1] = rec1;
    ((W22pRenderFn)(uintptr_t)G32(0x109e3d30))(mode, GBYTES(0x109e3dc0), verts, count, rec);
}

// FUNCTION: HGSIM 0x1009ef50
void __cdecl Sim_1009ef50(int32_t count, void *verts, uint32_t flags)
{
    uint8_t *v = BYTES(verts), *soft;
    uint32_t bits, saved, fog;
    int32_t i;

    switch (flags & 0xf000) {
    case 0x8000:
        W22P_CALL_10090f17(flags & 0xfff);
        return;
    case 0x9000:
        Sim_100ad96e((int32_t)(flags & 0xfff));
        return;
    case 0:
        flags &= 0xff;
        if (G32(0x1017b940) != 0) {
            for (i = 0; i < count; i++, v += 0x24) {
                W22p_Shade(v, flags);
                U32(v, 0x10) = 0xff;
                U32(v, 0x18) = 0xff000000u;
            }
            W22p_Render(0xd, verts, count, flags);
            return;
        }
        U32(GBYTES(0x101ce4a0), 8) = flags;
        bits = W22p_Soft(count, BYTES(verts), flags, 0);
        soft = GBYTES(0x101ce4a0);
        if (bits != 0) {
            count = W22P_CALL_100a2d91(count, bits);
            if (count > 2)
                soft = GBYTES(0x101ccca0);
        }
        if (count > 2)
            W22P_CALL_100e4780(GBYTES(0x109e3e00), count, soft);
        return;
    case 0x1000:
        flags &= 0xff;
        if (G32(0x1017b940) != 0) {
            for (i = 0; i < count; i++, v += 0x24) {
                W22p_Shade(v, U32(v, 0x10) + (flags & 0xf0));
                U32(v, 0x10) = 0xff;
            }
            W22p_Render(0xd, verts, count, 0);
            return;
        }
        bits = W22p_Soft(count, BYTES(verts), flags, 1);
        soft = GBYTES(0x101ce4a0);
        if (bits != 0) {
            count = W22P_CALL_100a2d91(count, bits);
            if (count > 2)
                soft = GBYTES(0x101ccca0);
        }
        if (count > 2)
            W22P_CALL_100e5477(GBYTES(0x109e3e00), 0x7fff, count, soft);
        return;
    case 0xa000:
        flags &= 0xff;
        if (G32(0x1017b940) != 0 && G32(0x1017b948) != 0) {
            int32_t c = GS32(0x10185760 + flags * 4);

            for (i = 0; i < count; i++, v += 0x24) {
                v[0x14] = (uint8_t)(c >> 16);
                v[0x15] = (uint8_t)(c >> 8);
                v[0x16] = (uint8_t)c;
                U32(v, 0x10) = 0xff;
                U32(v, 0x18) = 0xff000000u;
            }
            W22p_Render(0x10, verts, count, 0);
            return;
        }
        G32(0x101cc2bc) = (uint32_t)Render_Ftol(W22P_FL(v, 0));
        G32(0x101ccc6c) = (uint32_t)Render_Ftol(W22P_FL(v, 4));
        for (i = 1; i < count; i++)
            W22P_CALL_100a2b8c(Render_Ftol(W22P_FL(v + i * 0x24, 0)), Render_Ftol(W22P_FL(v + i * 0x24, 4)), flags);
        W22P_CALL_100a2b8c(Render_Ftol(W22P_FL(v, 0)), Render_Ftol(W22P_FL(v, 4)), flags);
        return;
    case 0x4000:
        flags &= 0xff;
        for (i = 0; i < count; i++, v += 0x24) {
            fog = (uint32_t)Render_Ftol((double)U32(v, 0x10) / (double)*SIM_PTR(w22pf, 0x1012ba14) *
                                            *SIM_PTR(double, 0x1012ba18) +
                                        *SIM_PTR(double, 0x1012ba20));
            U32(v, 0x10) = G32(0x1017d108) != 0 ? fog : 0xff - fog;
        }
        W22p_Render(0xd, verts, count, flags);
        return;
    case 0x3000:
        W22P_CALL_10098f30(flags, count, verts, -1);
        return;
    case 0x5000:
        if (G32(0x1014caf4) != 0)
            W22P_CALL_10006609(flags & 0xff, count, verts, 0, 0, 1, 1);
        return;
    case 0x6000:
        if (G32(0x1014caf4) != 0) {
            saved = G32(0x1014cb34);
            G32(0x1014cb34) = 1;
            W22P_CALL_10006609(flags & 0xff, count, verts, 0, 0, 1, 1);
            G32(0x1014cb34) = saved;
        }
        return;
    case 0x2000:
    case 0x7000:
        if (G32(0x1014caf4) != 0)
            W22P_CALL_10006744(flags & 0xff, count, verts, 0);
        return;
    case 0xb000:
    case 0xc000:
    case 0xd000:
    case 0xe000:
        if (G32(0x1014caf4) != 0)
            W22P_CALL_10006744(flags & 0xff, count, verts, 1);
        return;
    default:
        return;
    }
}

#define W22P_CALL_100ba7d5 SIM_FN(void *(__cdecl *)(void *, uint32_t, uint32_t), 0x100ba7d5)
#define W22P_CALL_100a0339 SIM_FN(int(__cdecl *)(void *, void *, int32_t, int32_t), 0x100a0339)

/* x87: a float product plus a float, rounded once to float */
static float W22p_MulAdd(float a, float b, float c)
{
    return (float)((double)a * (double)b + (double)c);
}

/* x87: a float product minus a float, rounded once to float */
static float W22p_MulSub(float a, float b, float c)
{
    return (float)((double)a * (double)b - (double)c);
}

// FUNCTION: HGSIM 0x100a0880
void __cdecl Sim_100a0880(void)
{
    /* the four 0x58-byte clip vertices; +0..+8 and +0x20.. are never written by the original (DECISIONS.md round 22:
       nothing reads them before Render_ClipPolygon writes them) */
    uint8_t v[4 * 0x58];
    uint8_t *cam = BYTES(G32(0x1014cae0)), *k, *tex;
    float yaw, pitch, c1, s1, t, kk, r, rc, rs, top, bottom, dt, xmin = 3.4028235e38f, zmin = 3.4028235e38f, cell;
    int32_t ix, iz, m, i;
    uint32_t saved, n;

    if ((int8_t)*GBYTES(0x1017d0b0) == 0)
        return;
    if (G32(0x1017b940) == 0)
        Sim_100a88db();
    yaw = W22P_FL(cam, 0xc);
    pitch = (float)((double)*SIM_PTR(w22pf, 0x1012ba2c) - (double)W22P_FL(cam, 0x10));
    while (yaw > *SIM_PTR(w22pf, 0x1012ba30))
        yaw = (float)((double)yaw - (double)*SIM_PTR(w22pf, 0x1012ba30));
    while (!(yaw >= *SIM_PTR(w22pf, 0x1012ba28)))     /* fcomp / test ah, 1: less, or unordered */
        yaw = (float)((double)yaw + (double)*SIM_PTR(w22pf, 0x1012ba30));
    c1 = Sim_10084766((float)((double)yaw + (double)*SIM_PTR(w22pf, 0x1012ba34)));
    s1 = Sim_10084766(yaw);
    t = Sim_10084766(pitch);
    t = (float)((double)t / (double)Sim_10084766((float)((double)pitch + (double)*SIM_PTR(w22pf, 0x1012ba34))));
    kk = (float)((double)*SIM_PTR(w22pf, 0x1012ba38) / (double)t);
    r = *SIM_PTR(w22pf, 0x1014c9c8);
    rc = (float)((double)r * (double)c1);
    rs = (float)((double)r * (double)s1);

    memset(v, 0, sizeof(v));
    k = v;
    W22P_FL(k, 0x0c) = W22p_MulAdd(kk, s1, -rc);
    W22P_FL(k + 0x58, 0x0c) = W22p_MulAdd(kk, s1, rc);
    W22P_FL(k + 0xb0, 0x0c) = (float)((double)rs + (double)rc);
    W22P_FL(k + 0x108, 0x0c) = (float)((double)rs + (double)-rc);
    W22P_FL(k, 0x14) = W22p_MulSub(kk, c1, -rs);
    W22P_FL(k + 0x58, 0x14) = W22p_MulSub(kk, c1, rs);
    W22P_FL(k + 0xb0, 0x14) = (float)((double)rc - (double)rs);
    W22P_FL(k + 0x108, 0x14) = (float)((double)rc - (double)-rs);
    top = (float)((double)*SIM_PTR(w22pf, 0x1012ba38) + (double)W22P_FL(cam, 4));
    W22P_FL(k + 0x58, 0x10) = top;
    W22P_FL(k, 0x10) = top;
    bottom = (float)((double)W22P_FL(cam, 4) - (double)*SIM_PTR(w22pf, 0x1012ba3c));
    W22P_FL(k + 0x108, 0x10) = bottom;
    W22P_FL(k + 0xb0, 0x10) = bottom;

    for (i = 0; i < 4; i++) {
        if (!(W22P_FL(k + i * 0x58, 0xc) >= xmin))
            xmin = W22P_FL(k + i * 0x58, 0xc);
        if (!(W22P_FL(k + i * 0x58, 0x14) >= zmin))
            zmin = W22P_FL(k + i * 0x58, 0x14);
    }
    cell = *SIM_PTR(w22pf, 0x1014c9e8);
    m = Render_Ftol(cell);
    ix = Render_Ftol(xmin) - Render_Ftol(xmin) % m;
    iz = Render_Ftol(zmin) - Render_Ftol(zmin) % m;
    dt = (float)((double)GS32(0x1015dc80) * (double)*SIM_PTR(w22pf, 0x1014c9f8) /
                 (double)*SIM_PTR(w22pf, 0x1012ba40));
    *SIM_PTR(w22pf, 0x1017d104) = (float)((double)*SIM_PTR(w22pf, 0x1017d104) + (double)dt);
    *SIM_PTR(w22pf, 0x1014c9f4) = (float)((double)*SIM_PTR(w22pf, 0x1014c9f4) + (double)dt);
    if (*SIM_PTR(w22pf, 0x1017d104) >= *SIM_PTR(w22pf, 0x1012ba44)) {     /* test ah, 1: not less, ordered */
        G32(0x1017d104) = 0;
        G32(0x1014c9f4) = G32(0x1014c9c0);
    }
    for (i = 0; i < 4; i++) {
        uint8_t *p = k + i * 0x58;

        W22P_FL(p, 0x18) = (float)(((double)W22P_FL(p, 0xc) - (double)ix) / (double)cell +
                                   (double)*SIM_PTR(w22pf, 0x1017d104));
        W22P_FL(p, 0x1c) = (float)(((double)W22P_FL(p, 0x14) - (double)iz) / (double)cell +
                                   (double)*SIM_PTR(w22pf, 0x1017d104));
    }
    for (i = 0; i < 4; i++)
        W22P_FL(k + i * 0x58, 0xc) = (float)((double)W22P_FL(k + i * 0x58, 0xc) + (double)W22P_FL(cam, 0));
    for (i = 0; i < 4; i++)
        W22P_FL(k + i * 0x58, 0x14) = (float)((double)W22P_FL(k + i * 0x58, 0x14) + (double)W22P_FL(cam, 8));
    for (i = 0; i < 4; i++)
        U32(k + i * 0x58, 0x2c) = 0;

    saved = G32(0x1014cb34);
    G32(0x1014cb34) = 0;
    n = Render_ClipPolygon(v, 4, G32(0x10185c50), 0x4caba950u);
    if (n != 0) {
        if (G32(0x1017d100) == 0 && (int8_t)*GBYTES(0x1017d0b0) != 0) {
            tex = BYTES(W22P_CALL_100ba7d5(GBYTES(0x1017d0b0), G32(0x1014ceb8), 0));
            G32(0x1017d100) = (uint32_t)(uintptr_t)tex;
            if (tex != 0) {
                G32(0x101cfca0) = (uint32_t)(int32_t)S16(tex, 0);
                G32(0x101cfca4) = (uint32_t)(int32_t)S16(tex, 2);
                G32(0x101cfca8) = (uint32_t)(uintptr_t)(tex + 4);
                G32(0x101cfcb8) = (uint32_t)(uintptr_t)GBYTES(0x101cfca0);
            }
        }
        if (G32(0x1017d100) != 0)
            W22P_CALL_100a0339(GBYTES(0x101cfcb8), GBYTES(0x101c76a0), (int32_t)n, 0);
    }
    G32(0x1014cb34) = saved;
}

#define W22P_CALL_1006018d SIM_FN(void(__cdecl *)(void *), 0x1006018d)
#define W22P_CALL_1005fbb0 SIM_FN(void(__cdecl *)(void *, void *, void *, void *), 0x1005fbb0)
#define W22P_CALL_10052e60 SIM_FN(int32_t(__cdecl *)(int32_t, int32_t, void *), 0x10052e60)
#define W22P_CALL_10052f3a SIM_FN(int32_t(__cdecl *)(int32_t, void *), 0x10052f3a)
#define W22P_CALL_10052edc SIM_FN(int32_t(__cdecl *)(int32_t, void *), 0x10052edc)
#define W22P_CALL_1000656f SIM_FN(void(__cdecl *)(void), 0x1000656f)
#define W22P_QUAD SIM_FN(int(__cdecl *)(int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, \
                                         uint32_t), 0x100a291d)
#define W22P_TRI SIM_FN(int(__cdecl *)(int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, uint32_t), 0x100a29da)
#define W22P_PENTA SIM_FN(int(__cdecl *)(int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, \
                                          int32_t, int32_t, uint32_t), 0x100a2823)
#define W22P_GROUND (G32(0x1014cb08) != 0 ? G32(0x10138c78) : 0)
#define W22P_SKY G32(0x10138c74)

/* the four corners of the haze band as screen integers (x / y of each 0x18-byte software vertex) */
struct W22pHaze {
    int32_t x0, y0, x1, y1, x2, y2, x3, y3;
};

/* a quad from the screen rectangle record (a copy of 0x109e3e00) moved by (dx, dy), w + ew wide, h + eh high */
static void W22p_RectQuad(int32_t dx, int32_t dy, int32_t w, int32_t h, uint32_t colour)
{
    int32_t s[5], x2, y2;

    memcpy(s, GBYTES(0x109e3e00), sizeof(s));
    s[1] += dx;
    s[2] += dy;
    x2 = s[1] + w;
    y2 = s[2] + h;
    W22P_QUAD(s[1], s[2], s[1], y2, x2, y2, x2, s[2], colour);
}

// FUNCTION: HGSIM 0x100a0def
void __cdecl Sim_100a0def(void *view)
{
    int32_t left = GS32(0x10185bd0), right = GS32(0x10185bec), top = GS32(0x10185bf0), bottom = GS32(0x10185be0);
    float m[12], fa = 0, fb = (float)GS32(0x1014cb50), fc = 0, yaw, u;
    int32_t band, depth, corner, h14 = 0, ha8 = 0, hb4 = 0, ha4 = 0;
    struct W22pHaze z;
    int haze = 0;
    uint8_t pv[4 * 0x24];   /* the plugin vertices; their alpha bytes are never written by the original (0 here) */
    int32_t sw[4][6];       /* the software vertices; +0xc..+0x14 never written by the original (0 here) */
    uint32_t light0, light2;
    uint8_t *tex;
    int i;

    memset(&z, 0, sizeof(z));
    memset(pv, 0, sizeof(pv));
    memset(sw, 0, sizeof(sw));
    W22P_CALL_1006018d(m);
    m[0] = Sim_10084766((float)((double)W22P_FL(view, 0x14) + (double)*SIM_PTR(w22pf, 0x1012ba34)));
    m[4] = m[0];
    m[3] = Sim_10084766(W22P_FL(view, 0x14));
    m[1] = -m[3];
    m[9] = 0;
    m[10] = 0;
    m[11] = 0;
    W22P_CALL_1005fbb0(m, &fa, &fb, &fc);
    band = Render_Ftol(fa);
    depth = Render_Ftol(fb);
    (void)Render_Ftol(fc);
    corner = W22P_CALL_10052e60(left, top, view);
    corner += W22P_CALL_10052e60(right, top, view) * 2;
    corner += W22P_CALL_10052e60(left, bottom, view) * 4;
    corner += W22P_CALL_10052e60(right, bottom, view) * 8;
    if (G32(0x1012da34) != 0) {
        G32(0x1012da34) = 0;
        W22P_CALL_1000656f();
    }
    light0 = G32(0x1017b940) != 0 ? 0xff : 0x80;
    light2 = light0;
    yaw = W22P_FL(view, 0xc);
    while (!(yaw >= *SIM_PTR(w22pf, 0x1012ba28)))
        yaw = (float)((double)yaw + (double)*SIM_PTR(w22pf, 0x1012ba30));
    while (yaw > *SIM_PTR(w22pf, 0x1012ba30))
        yaw = (float)((double)yaw - (double)*SIM_PTR(w22pf, 0x1012ba30));
    {
        double d = (double)*SIM_PTR(w22pf, 0x1012ba4c) -
                   (double)yaw / (double)*SIM_PTR(w22pf, 0x1012ba30) * (double)*SIM_PTR(w22pf, 0x1012ba48);

        u = d > (double)*SIM_PTR(w22pf, 0x1012ba50) ? 0.0f : (float)d;
    }
    for (i = 0; i < 4; i++) {
        uint8_t *v = pv + i * 0x24;

        U32(v, 8) = 0x3f800000u;
        U32(v, 0xc) = 0x3f800000u;
        U32(v, 0x18) = 0xff000000u;
        v[0x14] = *GBYTES(0x10143fdc);
        v[0x15] = *GBYTES(0x10143fe0);
        v[0x16] = *GBYTES(0x10143fe4);
        W22P_FL(v, 0x1c) = i == 1 || i == 2 ? (float)((double)u + (double)*SIM_PTR(w22pf, 0x1014c9f0)) : u;
        U32(v, 0x20) = i < 2 ? 0x3c23d70au : 0x3f7d70a4u;
    }
    if (G32(0x1014cb38) != 0) {
        light0 = 0;
        light2 = 0x1f;
    }
    U32(pv, 0x10) = light0;
    U32(pv + 0x24, 0x10) = light0;
    U32(pv + 0x48, 0x10) = light2;
    U32(pv + 0x6c, 0x10) = light2;
    sw[0][2] = 0xc00000;
    sw[1][2] = 0xc00000;
    sw[2][2] = 0xde0000;
    sw[3][2] = 0xde0000;

    switch (corner) {
    case 0:
        W22p_RectQuad(left, top, right - left, bottom - top, W22P_GROUND);
        break;
    case 1:
        hb4 = W22P_CALL_10052f3a(top, view);
        h14 = W22P_CALL_10052edc(left, view);
        if (G32(0x1014cb04) != 0)
            W22P_TRI(left, top, left, h14, hb4, top, W22P_SKY);
        W22P_PENTA(left, bottom, right, bottom, right, top, hb4, top, left, h14, W22P_GROUND);
        break;
    case 2:
        hb4 = W22P_CALL_10052f3a(top, view);
        ha8 = W22P_CALL_10052edc(right, view);
        if (G32(0x1014cb04) != 0)
            W22P_TRI(right, top, hb4, top, right, ha8, W22P_SKY);
        W22P_PENTA(left, top, left, bottom, right, bottom, right, ha8, hb4, top, W22P_GROUND);
        break;
    case 3:
        h14 = W22P_CALL_10052edc(left, view);
        ha8 = W22P_CALL_10052edc(right, view);
        if (G32(0x1014cb0c) != 0 && G32(0x1014cb04) != 0) {
            z.x0 = right;
            z.y0 = ha8 - depth;
            z.x1 = left;
            z.y1 = h14 - depth;
            z.x2 = left;
            z.y2 = h14;
            z.x3 = right;
            z.y3 = ha8;
            haze = 1;
        }
        if (h14 == ha8) {
            if (G32(0x1014cb04) != 0)
                W22p_RectQuad(left, top, right - left, h14 - top, W22P_SKY);
            W22p_RectQuad(left, h14, right - left, bottom - h14 - 1, W22P_GROUND);
        } else {
            if (G32(0x1014cb04) != 0)
                W22P_QUAD(right, top, left, top, left, h14, right, ha8, W22P_SKY);
            if (G32(0x1014cb08) == 0)
                W22P_QUAD(left, bottom, right, bottom, right, ha8, left, h14, 0);
            else
                W22P_QUAD(left, bottom, right, bottom, right, ha8 - 1, left, h14 - 1, G32(0x10138c78));
        }
        break;
    case 4:
        ha4 = W22P_CALL_10052f3a(bottom, view);
        h14 = W22P_CALL_10052edc(left, view);
        if (G32(0x1014cb04) != 0)
            W22P_TRI(left, h14, left, bottom, ha4, bottom, W22P_SKY);
        W22P_PENTA(right, bottom, right, top, left, top, left, h14, ha4, bottom, W22P_GROUND);
        break;
    case 5:
        hb4 = W22P_CALL_10052f3a(top, view);
        ha4 = W22P_CALL_10052f3a(bottom, view);
        if (hb4 == ha4) {
            if (G32(0x1014cb04) != 0)
                W22p_RectQuad(left, top, hb4 - left - 1, bottom - top - 1, W22P_SKY);
            W22p_RectQuad(hb4 - left + 1, top, right - hb4 - 1, bottom - top - 1, W22P_GROUND);
        } else {
            if (G32(0x1014cb04) != 0)
                W22P_QUAD(left, top, left, bottom, ha4, bottom, hb4, top, W22P_SKY);
            W22P_QUAD(right, bottom, right, top, hb4, top, ha4, bottom, W22P_GROUND);
        }
        if (G32(0x1014cb0c) != 0 && G32(0x1014cb04) != 0) {
            z.x0 = hb4 - band;
            z.y0 = top;
            z.x1 = ha4 - band;
            z.y1 = bottom;
            z.x2 = ha4;
            z.y2 = bottom;
            z.x3 = hb4;
            z.y3 = top;
            haze = 1;
        }
        break;
    case 7:
        ha4 = W22P_CALL_10052f3a(bottom, view);
        ha8 = W22P_CALL_10052edc(right, view);
        h14 = W22P_CALL_10052edc(left, view);
        W22P_TRI(ha4, bottom, right, bottom, right, ha8, W22P_GROUND);
        if (G32(0x1014cb04) != 0) {
            W22P_PENTA(right, top, left, top, left, bottom, ha4, bottom, right, ha8, W22P_SKY);
            if (G32(0x1014cb0c) != 0) {
                z.x0 = right;
                z.y0 = ha8 - depth;
                z.x1 = left;
                z.y1 = h14 - depth;
                z.x2 = left;
                z.y2 = h14;
                z.x3 = right;
                z.y3 = ha8;
                haze = 1;
            }
        }
        break;
    case 8:
        ha4 = W22P_CALL_10052f3a(bottom, view);
        ha8 = W22P_CALL_10052edc(right, view);
        if (G32(0x1014cb04) != 0)
            W22P_TRI(ha4, bottom, right, bottom, right, ha8, W22P_SKY);
        W22P_PENTA(right, top, left, top, left, bottom, ha4, bottom, right, ha8, W22P_GROUND);
        break;
    case 10:
        hb4 = W22P_CALL_10052f3a(top, view);
        ha4 = W22P_CALL_10052f3a(bottom, view);
        if (hb4 == ha4) {
            W22p_RectQuad(left, top, hb4 - left - 1, bottom - top - 1, W22P_GROUND);
            if (G32(0x1014cb04) != 0)
                W22p_RectQuad(hb4 - left + 1, top, right - hb4 - 1, bottom - top - 1, W22P_SKY);
        } else {
            W22P_QUAD(left, top, left, bottom, ha4, bottom, hb4, top, W22P_GROUND);
            if (G32(0x1014cb04) != 0)
                W22P_QUAD(right, bottom, right, top, hb4, top, ha4, bottom, W22P_SKY);
        }
        if (G32(0x1014cb0c) != 0 && G32(0x1014cb04) != 0) {
            z.x0 = hb4 - band;
            z.y0 = top;
            z.x1 = ha4 - band;
            z.y1 = bottom;
            z.x2 = ha4;
            z.y2 = bottom;
            z.x3 = hb4;
            z.y3 = top;
            haze = 1;
        }
        break;
    case 0xb:
        ha4 = W22P_CALL_10052f3a(bottom, view);
        h14 = W22P_CALL_10052edc(left, view);
        ha8 = W22P_CALL_10052edc(right, view);
        W22P_TRI(left, h14, left, bottom, ha4, bottom, W22P_GROUND);
        if (G32(0x1014cb04) != 0) {
            W22P_PENTA(right, bottom, right, top, left, top, left, h14, ha4, bottom, W22P_SKY);
            if (G32(0x1014cb0c) != 0) {
                z.x0 = right;
                z.y0 = ha8 - depth;
                z.x1 = left;
                z.y1 = h14 - depth;
                z.x2 = left;
                z.y2 = h14;
                z.x3 = right;
                z.y3 = ha8;
                haze = 1;
            }
        }
        break;
    case 0xc:
        h14 = W22P_CALL_10052edc(left, view);
        ha8 = W22P_CALL_10052edc(right, view);
        if (G32(0x1014cb0c) != 0 && G32(0x1014cb04) != 0) {
            z.x1 = right - band;
            z.x0 = left + band;
            z.y3 = h14;
            z.y0 = h14 - depth;
            z.y1 = ha8 - depth;
            haze = 1;
            z.x2 = z.x1;
            z.y2 = ha8;
            z.x3 = z.x0;
        }
        if (h14 == ha8) {
            W22p_RectQuad(left, top, right - left, h14 - top, W22P_GROUND);
            if (G32(0x1014cb04) != 0)
                W22p_RectQuad(left, h14, right - left, bottom - h14, W22P_SKY);
        } else {
            W22P_QUAD(right, top, left, top, left, h14, right, ha8, W22P_GROUND);
            if (G32(0x1014cb04) != 0)
                W22P_QUAD(left, bottom, right, bottom, right, ha8, left, h14, W22P_SKY);
        }
        break;
    case 0xd:
        hb4 = W22P_CALL_10052f3a(top, view);
        ha8 = W22P_CALL_10052edc(right, view);
        W22P_TRI(right, top, hb4, top, right, ha8, W22P_GROUND);
        if (G32(0x1014cb04) != 0)
            W22P_PENTA(left, top, left, bottom, right, bottom, right, ha8, hb4, top, W22P_SKY);
        break;
    case 0xe:
        hb4 = W22P_CALL_10052f3a(top, view);
        h14 = W22P_CALL_10052edc(left, view);
        W22P_TRI(left, top, left, h14, hb4, top, W22P_GROUND);
        if (G32(0x1014cb04) != 0)
            W22P_PENTA(left, bottom, right, bottom, right, top, hb4, top, left, h14, W22P_SKY);
        break;
    case 0xf:
        if (G32(0x1014cb04) != 0) {
            if (G32(0x1014cb0c) != 0 && W22P_CALL_10052e60(left, bottom + depth, view) == 0) {
                h14 = W22P_CALL_10052edc(left, view);
                ha8 = W22P_CALL_10052edc(right, view);
                if (ha8 - depth < bottom || h14 - depth < bottom) {
                    z.x0 = right;
                    z.y0 = ha8 - depth;
                    z.x1 = left;
                    z.y1 = h14 - depth;
                    z.x2 = left;
                    z.y2 = h14;
                    z.x3 = right;
                    z.y3 = ha8;
                    haze = 1;
                }
            }
            W22p_RectQuad(left, top, right - left, bottom - top, W22P_SKY);
        }
        break;
    default:
        break;
    }

    if (haze && G32(0x1014caec) != 0 && G32(0x1017d0ac) == 0) {
        sw[0][0] = z.x0;
        sw[0][1] = z.y0;
        sw[1][0] = z.x1;
        sw[1][1] = z.y1;
        sw[2][0] = z.x2;
        sw[2][1] = z.y2;
        sw[3][0] = z.x3;
        sw[3][1] = z.y3;
        if (G32(0x1014cb38) != 0 && G32(0x1017b940) == 0) {
            W22P_CALL_100e5477(GBYTES(0x109e3e00), 0x8000, 4, sw);
        } else {
            for (i = 0; i < 4; i++) {
                W22P_FL(pv + i * 0x24, 0) = (float)sw[i][0];
                W22P_FL(pv + i * 0x24, 4) = (float)sw[i][1];
            }
            if (G32(0x1017d0fc) == 0 && (int8_t)*GBYTES(0x1017d0d0) != 0) {
                tex = BYTES(W22P_CALL_100ba7d5(GBYTES(0x1017d0d0), G32(0x1014ceb8), 0));
                G32(0x1017d0fc) = (uint32_t)(uintptr_t)tex;
                if (tex != 0) {
                    G32(0x101ccc80) = (uint32_t)(int32_t)S16(tex, 0);
                    G32(0x101ccc84) = (uint32_t)(int32_t)S16(tex, 2);
                    G32(0x101ccc88) = (uint32_t)(uintptr_t)(tex + 4);
                    G32(0x101cfc90) = (uint32_t)(uintptr_t)GBYTES(0x101ccc80);
                }
            }
            if (G32(0x1017d0fc) != 0 && G32(0x1014cb38) == 0) {
                G32(0x101ccc8c) = (uint32_t)(uintptr_t)GBYTES(0x10185760);
                ((W22pRenderFn)(uintptr_t)G32(0x109e3d30))(2, GBYTES(0x109e3dc0), pv, 4, GBYTES(0x101cfc90));
            } else {
                ((W22pDriverFn)(uintptr_t)G32(0x1014cb4c))(4, pv, (G32(0x10138c74) & 0xf0) | 0x1000);
            }
        }
    }
    if (G32(0x1017b940) != 0 && U32(G32(0x1015d1b8), 0x38) != 0)
        Sim_100a0880();
}
