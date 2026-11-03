#include "w10.h"
#include "../simmem.h"
#include "../util/access.h"
#include "../render/w2n.h"
#include "w3q.h"
#include "../unsorted_100a/w5c.h"
#include "../unsorted_1004/w9f.h"
#include "../unsorted_100f/w2e.h"
#include "../unsorted_100f/w8z.h"
#include "../unsorted_100a/w12.h"
#include "../libm/hg_math.h"
#include "../crt/hg_winheap.h"

#define W10_HEAPFREE(block) \
    Hg_HeapFree(G32(0x1017b8f8), 1, (void *)(uintptr_t)(block))

// FUNCTION: HGSIM 0x1005b197
void __cdecl Sim_1005b197(void *rec)
{
    if (U32(rec, 0x131) != 0)
        W10_HEAPFREE(U32(rec, 0x131));
    W10_HEAPFREE(rec);
}

#define GF(addr) (*SIM_PTR(float, addr))
#define LIGHT_X (*SIM_PTR(float, 0x109b16c4))
#define LIGHT_Y (*SIM_PTR(float, 0x109b16c8))
#define LIGHT_Z (*SIM_PTR(float, 0x109b16c0))

// FUNCTION: HGSIM 0x100591ff
int32_t __cdecl Sim_100591ff(const void *poly, const void *verts, int32_t b, float d)
{
    int32_t level = GS32(0x101668b4);
    int done = 0;
    int view3 = G32(0x1014cb1c) == 3;
    uint32_t sx = G32(0x109b16c4), sy = G32(0x109b16c8), sz = G32(0x109b16c0), sflag = G32(0x109afdcc);
    int32_t t;
    uint32_t e;

    if (view3 || G32(0x101668d8) != 0) {
        const float *cam = (const float *)(uintptr_t)G32(0x1014cae0);

        LIGHT_X = cam[0] - GF(0x101668c0);
        LIGHT_Y = cam[1] - GF(0x101668c4);
        LIGHT_Z = cam[2] - GF(0x101668c8);
        G32(0x109afdcc) = 0;
    } else if (G32(0x109afdcc) == 0) {
        LIGHT_X = LIGHT_X - GF(0x101668c0);
        LIGHT_Y = LIGHT_Y - GF(0x101668c4);
        LIGHT_Z = LIGHT_Z - GF(0x101668c8);
    } else {
        G32(0x109b16c4) = G32(0x10143fc0);
        G32(0x109b16c8) = G32(0x101668b8);
        G32(0x109b16c0) = G32(0x101668bc);
    }
    if (G32(0x10143fd4) != 0 || view3 || G32(0x101668d8) != 0) {
        t = Render_Ftol((double)(int32_t)(G32(0x10143fbc) * (uint32_t)Sim_100a6414(poly, verts)) /
                        (double)GF(0x1012b4b0));
        if (t > 0)
            level = (int32_t)((uint32_t)level + (uint32_t)t);
    }
    G32(0x109afdcc) = 0;
    if ((double)GS32(0x1014cb2c) > (double)GF(0x1012b4b4) && !view3) {
        level = (int32_t)((uint32_t)level - (uint32_t)(Render_Ftol((double)d / (double)GS32(0x1014cb2c)) >> 4));
        if (level < 0)
            level = 0;
    }
    if (G32(0x10143fd8) != 0 && level < 0x80 && !view3) {
        for (e = G32(0x101668d0); e != 0 && !done; e = U32(e, 0)) {
            if (Sim_1005897b(poly, verts, (const void *)(uintptr_t)e) == 0)
                continue;
            G32(0x109b16c4) = U32(e, 0x14);
            G32(0x109b16c8) = U32(e, 0x18);
            G32(0x109b16c0) = U32(e, 0x1c);
            t = Sim_100a6414(poly, verts);
            if ((double)t > (double)GF(0x1012b4b4)) {
                level = (int32_t)((uint32_t)level + (uint32_t)t);
                if (level >= 0x80)
                    done = 1;
            }
        }
    }
    G32(0x109b16c4) = sx;
    G32(0x109b16c8) = sy;
    G32(0x109b16c0) = sz;
    G32(0x109afdcc) = sflag;
    level = (int32_t)((uint32_t)level * (uint32_t)(b >> 1)) / 0x440;
    if (level < 1)
        level = 0;
    else if (level > 0xf)
        level = 0xf;
    return level;
}

#define W10_HEAPALLOC(size) \
    Hg_HeapAlloc(G32(0x1017b8f8), 1, (uint32_t)(size))

/* The original allocates on [0x1017b910] when it is set and puts [0x1017b900] back afterwards. */
static void *W10_ModelAlloc(uint32_t size)
{
    void *p;

    if (G32(0x1017b910) != 0)
        G32(0x1017b8f8) = G32(0x1017b910);
    p = W10_HEAPALLOC(size);
    G32(0x1017b8f8) = G32(0x1017b900);
    return p;
}

// FUNCTION: HGSIM 0x10059a34
void *__cdecl Sim_10059a34(const uint8_t *buf, uint32_t size)
{
    uint8_t *obj, *parts;
    uint32_t off = 0, word;
    int32_t i;

    if (buf == 0)
        return 0;
    obj = (uint8_t *)W10_ModelAlloc(0xd4);
    Mem_Move(&word, buf + off, 4);
    if (word != 0) {
        if (G32(0x1017b910) != 0)
            G32(0x1017b8f8) = G32(0x1017b910);
        W10_HEAPFREE(obj);
        G32(0x1017b8f8) = G32(0x1017b900);
        return 0;
    }
    Mem_Move(obj, buf + off, 0x88);
    off += 0x88;
    if (U32(obj, 0x84) != 0) {
        U32(obj, 0x88) = (uint32_t)(uintptr_t)W10_ModelAlloc(U32(obj, 0x84) * 0x438u);
        Mem_Move(BYTES(U32(obj, 0x88)), buf + off, U32(obj, 0x84) * 0x438u);
        off += U32(obj, 0x84) * 0x438u;
        for (i = 0; i < S32(obj, 0x84); i++) {
            parts = BYTES(U32(obj, 0x88));
            U32(parts, i * 0x438 + 0x10) = Sim_1004b8b0(U32(parts, i * 0x438 + 8));
            if (U32(parts, i * 0x438 + 0x78) != 0)
                U32(parts, i * 0x438 + 0x78) = U32(obj, 0x88) + U32(parts, i * 0x438 + 0x78) * 0x438u;
            if (U32(parts, i * 0x438 + 0x54) != 0)
                U32(parts, i * 0x438 + 0x54) = U32(obj, 0x88) + U32(parts, i * 0x438 + 0x54) * 0x438u;
        }
    }
    Mem_Move(&word, buf + off, 4);
    if (word != 1 && word != 2) {
        W10_HEAPFREE(U32(obj, 0x88));
        W10_HEAPFREE(obj);
        return 0;
    }
    Mem_Move(obj + 0x8c, buf + off, 0x3c);
    off += 0x3c;
    if (U32(obj, 0xb0) != 0) {
        U32(obj, 0xc8) = (uint32_t)(uintptr_t)W10_ModelAlloc(U32(obj, 0xb0) * 0x6cu);
        Mem_Move(BYTES(U32(obj, 0xc8)), buf + off, U32(obj, 0xb0) * 0x6cu);
        off += U32(obj, 0xb0) * 0x6cu;
    }
    if (off == size) {
        U32(obj, 0xcc) = 0;
        U32(obj, 0xd0) = 0;
        return obj;
    }
    Mem_Move(obj + 0xcc, buf + off, 4);
    off += 4;
    U32(obj, 0xd0) = (uint32_t)(uintptr_t)W10_ModelAlloc(U32(obj, 0xcc));
    Mem_Move(BYTES(U32(obj, 0xd0)), buf + off, U32(obj, 0xcc));
    return obj;
}


typedef float(__cdecl *W10FogFn)(uint32_t z, uint32_t arg);

#define W10_V3C(v) S32(v, 0x3c)
/* Sim_100a648c with the vertex's position +0xc.. and normal +0x30.., as raw words */
#define W10_LIGHT(v) SIM_CALL_100a648c(U32(v, 0xc), U32(v, 0x10), U32(v, 0x14), U32(v, 0x30), U32(v, 0x34), U32(v, 0x38))

// FUNCTION: HGSIM 0x10058c5e
// FUNCTION: HGSIM12 0x1004bad6
/*
 * The light of a polygon's points (+0x3c: 0..0x1f), as 1.2 computes it: 1.0's light with, besides, a colour per
 * point in +0x3c's upper three bytes (red, green, blue: SKYGND.PAR's ambient, plus the global light's, light 1's and
 * light 2's share in their colours, plus white for every point light reaching it), and the point lights also in 3D
 * mode with fog on (not for the b000..e000 polygons).
 */
void __cdecl Sim_10058c5e(const uint8_t *poly, const uint8_t *verts, uint8_t *const *list, int32_t scale)
{
    int32_t rgb[0x80];          /* red, green, blue, light per point (1.2's 0x200-byte local) */
    uint32_t sx, sy, sz, sflag, kind;
    int mode3, near = 0, minv = 0, special;
    int32_t i, q, r, g, b;
    uint8_t *v;
    const uint8_t *src, *corner;

    if (list == 0 || poly == 0 || verts == 0)
        return;
    kind = U16(poly, 0) & 0xf000u;
    special = kind == 0xb000 || kind == 0xc000 || kind == 0xd000 || kind == 0xe000;
    for (i = 0; i < 0x80; i++)
        rgb[i] = 0;
    mode3 = G32(0x1014cb1c) == 3;
    sx = G32(0x109b16c4);
    sy = G32(0x109b16c8);
    sz = G32(0x109b16c0);
    sflag = G32(0x109afdcc);
    if (mode3 || G32(0x101668d8) != 0) {
        LIGHT_X = (float)((double)*(float *)BYTES(G32(0x1014cae0)) - GF(0x101668c0));
        LIGHT_Y = (float)((double)*(float *)(BYTES(G32(0x1014cae0)) + 4) - GF(0x101668c4));
        LIGHT_Z = (float)((double)*(float *)(BYTES(G32(0x1014cae0)) + 8) - GF(0x101668c8));
        G32(0x109afdcc) = 0;
    } else if (G32(0x109afdcc) == 0) {
        LIGHT_X = (float)((double)LIGHT_X - GF(0x101668c0));
        LIGHT_Y = (float)((double)LIGHT_Y - GF(0x101668c4));
        LIGHT_Z = (float)((double)LIGHT_Z - GF(0x101668c8));
    } else {
        G32(0x109b16c4) = G32(0x10143fc0);
        G32(0x109b16c8) = G32(0x101668b8);
        G32(0x109b16c0) = G32(0x101668bc);
    }
    for (i = 0; i < GS32(0x101c9ea8); i++) {
        v = list[i];
        if ((U8(v, 0x29) & 2) != 0)
            continue;
        U32(v, 0x3c) = G32(0x101668b4);
        rgb[i * 4] = g_w33Ambient[0];
        rgb[i * 4 + 1] = g_w33Ambient[1];
        rgb[i * 4 + 2] = g_w33Ambient[2];
        rgb[i * 4 + 3] = W10_V3C(v);
        if (G32(0x10143fd4) != 0 || mode3 || G32(0x101668d8) != 0) {
            q = Render_Ftol((double)(int32_t)(G32(0x10143fbc) * (uint32_t)W10_LIGHT(v)) / GF(0x1012b4b0));
            r = q * g_w33Global[0] >> 7;
            g = q * g_w33Global[1] >> 7;
            b = q * g_w33Global[2] >> 7;
            if (q > 0) {
                U32(v, 0x3c) += (uint32_t)q;
                rgb[i * 4] += r;
                rgb[i * 4 + 1] += g;
                rgb[i * 4 + 2] += b;
            }
        }
        if (G32(0x1017d108) != 0)
            U8(v, 0x2b) = (uint8_t)Render_Ftol((double)SIM_FN(W10FogFn, 0x10058ac2)(U32(v, 8), U32(v, 0x2c)) *
                                               GF(0x10143fcc));
    }
    /* 1.2: the two coloured lights, colour only, their directions already in the camera's frame */
    G32(0x109afdcc) = 1;
    for (i = 0; i < GS32(0x101c9ea8); i++) {
        v = list[i];
        if ((U8(v, 0x29) & 2) != 0)
            continue;
        LIGHT_X = g_w33Light1Cam[0];
        LIGHT_Y = g_w33Light1Cam[1];
        LIGHT_Z = g_w33Light1Cam[2];
        q = Render_Ftol((double)(int32_t)((uint32_t)W10_LIGHT(v) * 0xffu) / GF(0x1012b4b0));
        r = q * g_w33Light1[0] >> 7;
        g = q * g_w33Light1[1] >> 7;
        b = q * g_w33Light1[2] >> 7;
        if (q > 0) {
            rgb[i * 4] += r;
            rgb[i * 4 + 1] += g;
            rgb[i * 4 + 2] += b;
        }
        LIGHT_X = g_w33Light2Cam[0];
        LIGHT_Y = g_w33Light2Cam[1];
        LIGHT_Z = g_w33Light2Cam[2];
        q = Render_Ftol((double)(int32_t)((uint32_t)W10_LIGHT(v) * 0xffu) / GF(0x1012b4b0));
        r = q * g_w33Light2[0] >> 7;
        g = q * g_w33Light2[1] >> 7;
#ifdef W10L_MUTATE /* the checker's negative check only: light 2's blue from its green */
        b = q * g_w33Light2[1] >> 7;
#else
        b = q * g_w33Light2[2] >> 7;
#endif
        if (q > 0) {
            rgb[i * 4] += r;
            rgb[i * 4 + 1] += g;
            rgb[i * 4 + 2] += b;
        }
    }
    G32(0x109afdcc) = 0;
    /* 1.2: the point lights also in 3D mode with fog on, but not for the b000..e000 polygons; each adds white */
    if (G32(0x10143fd8) != 0 && !mode3 && !special) {
        for (src = BYTES(G32(0x101668d0)); src != 0; src = BYTES(U32(src, 0))) {
            if (Sim_1005897b(poly, verts, src) == 0)
                continue;
            corner = verts + poly[U32(poly, 4)] * 0x58u;
            LIGHT_X = (float)((double)*(const float *)(src + 0x20) - *(const float *)corner);
            LIGHT_Y = (float)((double)*(const float *)(src + 0x24) - *(const float *)(corner + 4));
            LIGHT_Z = (float)((double)*(const float *)(src + 0x28) - *(const float *)(corner + 8));
            /* below [0x1012b4b4] on all three axes, an unordered compare counting as below */
            near = !(hg_fabs((double)LIGHT_X) >= GF(0x1012b4b4)) && !(hg_fabs((double)LIGHT_Y) >= GF(0x1012b4b4)) &&
                   !(hg_fabs((double)LIGHT_Z) >= GF(0x1012b4b4));
            for (i = 0; i < GS32(0x101c9ea8); i++) {
                v = list[i];
                if ((U8(v, 0x29) & 2) != 0 || W10_V3C(v) >= 0x80)
                    continue;
                q = near ? 0x80 : W10_LIGHT(v);
                if (q > 0) {
                    U32(v, 0x3c) += (uint32_t)q;
                    rgb[i * 4] += 0xff;
                    rgb[i * 4 + 1] += 0xff;
                    rgb[i * 4 + 2] += 0xff;
                }
            }
        }
    }
    G32(0x109b16c4) = sx;
    G32(0x109b16c8) = sy;
    G32(0x109b16c0) = sz;
    G32(0x109afdcc) = sflag;
    if (G32(0x1014cb38) == 0)
        minv = 4;
    for (i = 0; i < GS32(0x101c9ea8); i++) {
        v = list[i];
        if ((U8(v, 0x29) & 2) != 0)
            continue;
        U32(v, 0x3c) <<= 1;
        if (W10_V3C(v) > 0x80)
            U32(v, 0x3c) = 0x80;
        U32(v, 0x3c) *= (uint32_t)scale;
        U32(v, 0x3c) = (uint32_t)(W10_V3C(v) / 1024);
        if (W10_V3C(v) < minv)
            U32(v, 0x3c) = (uint32_t)minv;
        if (W10_V3C(v) > 0x1f)
            U32(v, 0x3c) = 0x1f;
        r = rgb[i * 4];
        g = rgb[i * 4 + 1];
        b = rgb[i * 4 + 2];
        if (r < 0)
            r = 0;
        if (r > 0xff)
            r = 0xff;
        if (g < 0)
            g = 0;
        if (g > 0xff)
            g = 0xff;
        if (b < 0)
            b = 0;
        if (b > 0xff)
            b = 0xff;
        U32(v, 0x3c) |= (uint32_t)r << 8 | (uint32_t)g << 16 | (uint32_t)b << 24;
        U8(v, 0x29) |= 2;
    }
}
