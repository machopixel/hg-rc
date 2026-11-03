#include "w8z.h"
#include "../fx/w32.h"
#include "../util/util.h"
#include "../libm/hg_math.h"

// FUNCTION: HGSIM 0x100d2669
int __cdecl Sim_100d2669(float *x, float *y, float *z)
{
    float dx = (float)((double)*x - (double)W8Z_GF(0x10185c48));
    float dy = (float)((double)*y - (double)W8Z_GF(0x10185c44));
    float dz = (float)((double)*z - (double)W8Z_GF(0x10185c4c));
    float vx, vy, depth;
    int behind;
    int32_t sx, sy;
    uint32_t top;

    vx = (float)(((double)dx * (double)W8Z_GF(0x10185bfc) + (double)dy * (double)W8Z_GF(0x10185c00)) +
                 (double)dz * (double)W8Z_GF(0x10185c04));
    vy = (float)(((double)dx * (double)W8Z_GF(0x10185c08) + (double)dy * (double)W8Z_GF(0x10185c0c)) +
                 (double)dz * (double)W8Z_GF(0x10185c10));
    depth = (float)(((double)dx * (double)W8Z_GF(0x10185c14) + (double)dy * (double)W8Z_GF(0x10185c18)) +
                    (double)dz * (double)W8Z_GF(0x10185c1c));
    *z = depth;
    if (!(depth > W8Z_GF(0x10185c50))) {
        behind = 1;
        if ((W8z_Bits(depth) & 0x80000000u) != 0)
            depth = -depth;
        else if (!(hg_fabs((double)depth) >= (double)W8Z_GF(0x1012bcc4)))
            depth = 1.0f;
    } else {
        behind = 0;
    }
    sx = (int32_t)((uint32_t)W8z_Ftol((double)vx / (double)depth) + G32(0x10185bd4));
    top = G32(0x10185be0) - G32(0x10185bf0);
    sy = (int32_t)(top - ((uint32_t)W8z_Ftol((double)vy / (double)depth) + G32(0x10185bf4)));
    *x = (float)sx;
    *y = (float)sy;
    if (behind)
        return 0;
    return sx >= GS32(0x10185bd0) && sx <= GS32(0x10185bec) && sy >= GS32(0x10185bf0) && sy <= GS32(0x10185be0);
}

/* The colour word of index i (0x101659e0, two bytes each). */
static uint32_t W8z_ColourWord(int32_t i)
{
    return U16((uintptr_t)SIM_PTR(uint8_t, 0x101659e0) + (uint32_t)i * 2u, 0);
}

// FUNCTION: HGSIM 0x100d8e43
int __cdecl Sim_100d8e43(void *obj)
{
    uint32_t ox, oy, oz;
    float vx, vy, vz;
    float x0, y0, z0;
    float x1, y1, z1;
    uint32_t point;
    int32_t colour;
    int i;

    if (obj == 0)
        return 0;
    Sim_100a9cfb((void *)(uintptr_t)U32(obj, W32_REC_OBJ), &ox, &oy, &oz);
    vx = (float)((double)W8Z_FL(obj, W32_REC_VEL) / (double)W8Z_GF(0x1012bd6c));
    vy = (float)((double)W8Z_FL(obj, W32_REC_VEL + 4) / (double)W8Z_GF(0x1012bd6c));
    vz = (float)((double)W8Z_FL(obj, W32_REC_VEL + 8) / (double)W8Z_GF(0x1012bd6c));
    for (i = 0; i < 2; i++) {
        point = (uint32_t)(uintptr_t)obj + (uint32_t)i * W32_REC_PT;     /* 1.2 0x100be2e3: 0x32 apart */
        x0 = (float)((double)W8Z_FL(point, 0x12) + (double)W8z_Float(ox));
        y0 = (float)((double)W8Z_FL(point, 0x16) + (double)W8z_Float(oy));
        z0 = (float)((double)W8Z_FL(point, 0x1a) + (double)W8z_Float(oz));
        if (Sim_100d2669(&x0, &y0, &z0) == 0)
            continue;
        x1 = (float)(((double)W8Z_FL(point, 0x12) + (double)W8z_Float(ox)) + (double)vx);
        y1 = (float)(((double)W8Z_FL(point, 0x16) + (double)W8z_Float(oy)) + (double)vy);
        z1 = (float)(((double)W8Z_FL(point, 0x1a) + (double)W8z_Float(oz)) + (double)vz);
        Sim_100d2669(&x1, &y1, &z1);
        colour = W8Z_CALL_100e85cb(SIM_PTR(uint8_t, 0x109e3e00), W8z_Ftol(x0), W8z_Ftol(y0));
        if (colour % 16 == 15)
            colour = (int32_t)((uint32_t)colour - 1u);
        else
            colour = (int32_t)((uint32_t)colour + 1u);
        W8Z_CALL_100ef727(SIM_PTR(uint8_t, 0x109e3e00), W8z_Ftol(x0), W8z_Ftol(y0), W8z_Ftol(x1), W8z_Ftol(y1), 0,
                          W8z_ColourWord(colour));
    }
    return 1;
}

// FUNCTION: HGSIM 0x100d943f
int __cdecl Sim_100d943f(void *obj)
{
    uint32_t ox, oy, oz;
    uint32_t rec;
    uint32_t table;
    float vx, vy, vz;
    float x0, y0, z0;
    float x1, y1, z1;
    int32_t colour;

    if (obj == 0)
        return 0;
    Sim_100a9cfb((void *)(uintptr_t)U32(obj, W32_REC_OBJ), &ox, &oy, &oz);
    rec = U32(obj, W32_REC_ARG);
    table = U32(rec, 0x20);
    vx = (float)((double)W8Z_FL(rec, 0x14) / (double)W8Z_GF(0x1012bd78));
    vy = (float)((double)W8Z_FL(rec, 0x18) / (double)W8Z_GF(0x1012bd78));
    vz = (float)((double)W8Z_FL(rec, 0x1c) / (double)W8Z_GF(0x1012bd78));
    x0 = (float)((double)W8Z_FL(obj, 0x12) + (double)W8z_Float(ox));
    y0 = (float)((double)W8Z_FL(obj, 0x16) + (double)W8z_Float(oy));
    z0 = (float)((double)W8Z_FL(obj, 0x1a) + (double)W8z_Float(oz));
    if (Sim_100d2669(&x0, &y0, &z0) != 0) {
        colour = W8z_Ftol(hg_fabs((double)z0) * (double)W8Z_GF(0x1012bd7c) / (double)W8Z_FL(G32(0x1014cae0), 0x40));
        if (colour < 0)
            colour = 0;
        else if (colour >= 0x20)
            colour = 0x1f;
        colour = S32(table, 0x18 + 4 * colour);
        x1 = (float)(((double)W8Z_FL(obj, 0x12) + (double)W8z_Float(ox)) + (double)vx);
        y1 = (float)(((double)W8Z_FL(obj, 0x16) + (double)W8z_Float(oy)) + (double)vy);
        z1 = (float)(((double)W8Z_FL(obj, 0x1a) + (double)W8z_Float(oz)) + (double)vz);
        Sim_100d2669(&x1, &y1, &z1);
        W8Z_CALL_100ef727(SIM_PTR(uint8_t, 0x109e3e00), W8z_Ftol(x0), W8z_Ftol(y0), W8z_Ftol(x1), W8z_Ftol(y1), 0,
                          W8z_ColourWord(colour));
    }
    return 1;
}
