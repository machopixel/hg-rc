#include "w3p.h"
#include "w2n.h"
#include "../util/access.h"
#include "../util/w3p.h"
#include "../unsorted_100a/w12.h"

#define MAX_POLY 255

typedef void(__cdecl *DrawFn)(unsigned count, void *vertices, int mode);

static float Dot3(const uint8_t *m, uint32_t o0, uint32_t o1, uint32_t o2, float a, float b, float c)
{
    return (float)((double)a * (double)F32(m, o0) + (double)b * (double)F32(m, o1) + (double)c * (double)F32(m, o2));
}

// FUNCTION: HGSIM 0x1005874a
void __cdecl Sim_1005874a(const void *matrix)
{
    const uint8_t *m = (const uint8_t *)matrix;
    float tx = F32(m, 0x24);
    float ty = F32(m, 0x28);
    float tz = F32(m, 0x2c);
    float x, y, z;
    uint8_t *node;

    G32(0x10143fc0) = G32(0x109b16c4);
    G32(0x101668b8) = G32(0x109b16c8);
    G32(0x101668bc) = G32(0x109b16c0);
    if (G32(0x109afdcc) == 0) {
        GF32(0x10143fc0) = (float)((double)GF32(0x10143fc0) - (double)tx);
        GF32(0x101668b8) = (float)((double)GF32(0x101668b8) - (double)ty);
        GF32(0x101668bc) = (float)((double)GF32(0x101668bc) - (double)tz);
    }
    x = Dot3(m, 0, 0xc, 0x18, GF32(0x10143fc0), GF32(0x101668b8), GF32(0x101668bc));
    y = Dot3(m, 4, 0x10, 0x1c, GF32(0x10143fc0), GF32(0x101668b8), GF32(0x101668bc));
    z = Dot3(m, 8, 0x14, 0x20, GF32(0x10143fc0), GF32(0x101668b8), GF32(0x101668bc));
    GF32(0x10143fc0) = x;
    GF32(0x101668b8) = y;
    GF32(0x101668bc) = z;
    /* 1.2 (0x1004b44a): SKYGND.PAR's two coloured lights' directions into the camera's frame too (no origin) */
#ifdef W3PL_MUTATE /* the checker's negative check only: light 1's z left out */
    g_w33Light1Cam[0] = Dot3(m, 0, 0xc, 0x18, g_w33Light1Dir[0], g_w33Light1Dir[1], 0.0f);
#else
    g_w33Light1Cam[0] = Dot3(m, 0, 0xc, 0x18, g_w33Light1Dir[0], g_w33Light1Dir[1], g_w33Light1Dir[2]);
#endif
    g_w33Light1Cam[1] = Dot3(m, 4, 0x10, 0x1c, g_w33Light1Dir[0], g_w33Light1Dir[1], g_w33Light1Dir[2]);
    g_w33Light1Cam[2] = Dot3(m, 8, 0x14, 0x20, g_w33Light1Dir[0], g_w33Light1Dir[1], g_w33Light1Dir[2]);
    g_w33Light2Cam[0] = Dot3(m, 0, 0xc, 0x18, g_w33Light2Dir[0], g_w33Light2Dir[1], g_w33Light2Dir[2]);
    g_w33Light2Cam[1] = Dot3(m, 4, 0x10, 0x1c, g_w33Light2Dir[0], g_w33Light2Dir[1], g_w33Light2Dir[2]);
    g_w33Light2Cam[2] = Dot3(m, 8, 0x14, 0x20, g_w33Light2Dir[0], g_w33Light2Dir[1], g_w33Light2Dir[2]);
    for (node = BYTES(G32(0x101668d0)); node != 0; node = BYTES(U32(node, 0))) {
        float dx = (float)((double)F32(node, 0x14) - (double)tx);
        float dy = (float)((double)F32(node, 0x18) - (double)ty);
        float dz = (float)((double)F32(node, 0x1c) - (double)tz);

        F32(node, 0x20) = Dot3(m, 0, 0xc, 0x18, dx, dy, dz);
        F32(node, 0x24) = Dot3(m, 4, 0x10, 0x1c, dx, dy, dz);
        F32(node, 0x28) = Dot3(m, 8, 0x14, 0x20, dx, dy, dz);
    }
    G32(0x101668c0) = U32(m, 0x24);
    G32(0x101668c4) = U32(m, 0x28);
    G32(0x101668c8) = U32(m, 0x2c);
}

// FUNCTION: HGSIM 0x1008fdd7
int __cdecl Sim_1008fdd7(const void *object)
{
    const uint8_t *obj = (const uint8_t *)object;
    float radius, dx, dy, dz, depth, side;
    uint32_t bits;

    if ((U16(obj, 0) & 0x1000) != 0)
        return 1;
    radius = F32(obj, 0x50);
    dx = (float)((double)F32(obj, 0x44) - (double)GF32(0x10185c48));
    dy = (float)((double)F32(obj, 0x48) - (double)GF32(0x10185c44));
    dz = (float)((double)F32(obj, 0x4c) - (double)GF32(0x10185c4c));
    depth = (float)((double)dx * (double)GF32(0x10185c38) + (double)dy * (double)GF32(0x10185c3c) + (double)dz * (double)GF32(0x10185c40));
    GF32(0x101c7aa0) = depth;
    if (X87_LessOrUnordered((double)depth + (double)radius, (double)GF32(0x10185c50)))
        return 4;
    if ((double)depth - (double)radius > (double)GF32(0x10185bf8))
        return 5;
    side = (float)((double)dx * (double)GF32(0x10185c20) + (double)dy * (double)GF32(0x10185c24) + (double)dz * (double)GF32(0x10185c28));
    bits = *(uint32_t *)&side;
    if ((bits & 0x80000000u) == 0)
        side = (float)((double)side - (double)GF32(0x1036f1c4));
    else
        side = (float)((double)GF32(0x1036f1c0) - (double)side);
    if ((double)side > (double)radius)
        return 6;
    side = (float)((double)dx * (double)GF32(0x10185c2c) + (double)dy * (double)GF32(0x10185c30) + (double)dz * (double)GF32(0x10185c34));
    bits = *(uint32_t *)&side;
    if ((bits & 0x80000000u) == 0)
        side = (float)((double)side - (double)GF32(0x1036f1c8));
    else
        side = (float)((double)GF32(0x1036f1cc) - (double)side);
    if ((double)side > (double)radius)
        return 7;
    return 0;
}

// FUNCTION: HGSIM 0x1009b47d
int __cdecl Sim_1009b47d(const void *object)
{
    const uint8_t *obj = (const uint8_t *)object;
    int result = 0;

    if ((U16(obj, 2) & 0xf00) == 0x100) {
        result = (S16(BYTES(PTRTAB(U16(obj, 0x24))), 0x1c) & 0x16) != 0;
    } else {
        uint32_t kind = U16(obj, 2) & 0xf0;

        if (kind == 0x30 || kind == 0x70)
            result = 1;
    }
    if (result == 0)
        result = Sim_1008fdd7(obj);
    return result;
}

// FUNCTION: HGSIM 0x100ad5b7
void __cdecl Sim_100ad5b7(float x, float y, float z, float half)
{
    uint8_t scene[4 * 0x58];
    uint8_t out[MAX_POLY * 0x24];
    uint8_t *camera = BYTES(G32(0x1014cae0));
    uint8_t *v = scene;
    unsigned n;
    unsigned i;

    F32(v, 0xc) = (float)((double)x - (double)half);
    F32(v, 0x14) = (float)((double)z + (double)half);
    F32(v, 0x10) = y;
    U32(v, 0x18) = 0;
    U32(v, 0x1c) = 0;
    U32(v, 0x2c) = 0;
    v += 0x58;
    F32(v, 0xc) = (float)((double)x + (double)half);
    F32(v, 0x14) = (float)((double)z + (double)half);
    F32(v, 0x10) = y;
    U32(v, 0x18) = 0;
    U32(v, 0x1c) = 0;
    U32(v, 0x2c) = 0;
    v += 0x58;
    F32(v, 0xc) = (float)((double)x + (double)half);
    F32(v, 0x14) = (float)((double)z - (double)half);
    F32(v, 0x10) = y;
    U32(v, 0x18) = 0;
    U32(v, 0x1c) = 0;
    U32(v, 0x2c) = 0;
    v += 0x58;
    F32(v, 0xc) = (float)((double)x - (double)half);
    F32(v, 0x14) = (float)((double)z - (double)half);
    F32(v, 0x10) = y;
    U32(v, 0x18) = 0;
    U32(v, 0x1c) = 0;
    U32(v, 0x2c) = 0;
    n = Render_ClipPolygon(scene, 4, U32(camera, 0x3c), U32(camera, 0x40));
    if (n >= 3) {
        uint8_t **list = (uint8_t **)SIM_PTR(u32a, 0x101c76a0);
        uint8_t *o = out;

        for (i = 0; i < n; i++, o += 0x24) {
            const uint8_t *p = list[i];

            F32(o, 0) = (float)S32(p, 0x18);
            F32(o, 4) = (float)S32(p, 0x1c);
            U32(o, 8) = U32(p, 8);
            U32(o, 0xc) = 0x3f800000u;
            U32(o, 0x1c) = 0x3f800000u;
            U32(o, 0x20) = 0x3f800000u;
            U32(o, 0x10) = 0xf0;
            U32(o, 0x14) = (uint32_t)Render_FogLevel(F32(p, 8), y) << 24;
        }
        (*SIM_PTR(DrawFn, 0x1014cb4c))(n, out, 0xf0);
    }
}
