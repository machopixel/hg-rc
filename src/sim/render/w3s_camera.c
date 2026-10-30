#include "w3s.h"
#include "w3s_fp.h"
#include "../unsorted_1005/w2f.h"

#define COPY(dst, src) (G32(dst) = (src))

// FUNCTION: HGSIM 0x1008ff68
void *__cdecl Sim_1008ff68(unsigned char *vertex)
{
    unsigned char clip = 0;

    if (vertex[0x29] == 0) {
        int32_t x = (int32_t)((uint32_t)Render_Ftol((double)F32(vertex, 0) / (double)GF32(0x1036f1d8)) + G32(0x10185bd4));
        int32_t y = (int32_t)(G32(0x10185be0) - G32(0x10185bf0) -
                              ((uint32_t)Render_Ftol((double)F32(vertex, 4) / (double)GF32(0x1036f1d8)) + G32(0x10185bf4)));

        if ((int32_t)((uint32_t)x - G32(0x10185bd0)) < 0)
            clip |= 1;
        if ((int32_t)((uint32_t)x - G32(0x10185bec)) > 0)
            clip |= 2;
        if ((int32_t)((uint32_t)y - G32(0x10185bf0)) < 0)
            clip |= 4;
        if ((int32_t)((uint32_t)y - G32(0x10185be0)) > 0)
            clip |= 8;
        U32(vertex, 0x18) = (uint32_t)x;
        U32(vertex, 0x1c) = (uint32_t)y;
        vertex[0x28] = clip;
        vertex[0x29] = 1;
    }
    *SIM_PTR(unsigned char, 0x101c7680) |= clip;
    *SIM_PTR(unsigned char, 0x101cc2a0) &= clip;
    if (GS32(0x101c9ea8) < 0xff) {
        G32(0x101c76a0 + (uint32_t)GS32(0x101c9ea8) * 4u) = (uint32_t)(uintptr_t)vertex;
        G32(0x101c9ea8) = G32(0x101c9ea8) + 1;
    } else {
        G32(0x101c9ea4) = 0;
    }
    return vertex;
}

// FUNCTION: HGSIM 0x100d1f90
void __cdecl Sim_100d1f90(void *camera)
{
    float factor;
    float k = GF32(0x1012bca8);

    G32(0x1014cae0) = (uint32_t)(uintptr_t)camera;
    Sim_1005873d((uint32_t)(int32_t)S16(camera, 0x2a));
    COPY(0x10185be4, U32(camera, 0x94));
    COPY(0x10185be8, U32(camera, 0x98));
    COPY(0x10185c20, U32(camera, 0x54));
    COPY(0x10185c24, U32(camera, 0x58));
    COPY(0x10185c28, U32(camera, 0x5c));
    COPY(0x10185c2c, U32(camera, 0x60));
    COPY(0x10185c30, U32(camera, 0x64));
    COPY(0x10185c34, U32(camera, 0x68));
    COPY(0x10185c38, U32(camera, 0x6c));
    COPY(0x10185c14, G32(0x10185c38));
    COPY(0x10185bb8, G32(0x10185c14));
    COPY(0x10185c3c, U32(camera, 0x70));
    COPY(0x10185c18, G32(0x10185c3c));
    COPY(0x10185bbc, G32(0x10185c18));
    COPY(0x10185c40, U32(camera, 0x74));
    COPY(0x10185c1c, G32(0x10185c40));
    COPY(0x10185bc0, G32(0x10185c1c));
    COPY(0x10185c48, U32(camera, 0x78));
    COPY(0x10185c44, U32(camera, 0x7c));
    COPY(0x10185c4c, U32(camera, 0x80));
    Sim_10058716(U32(camera, 0x1c), U32(camera, 0x20), U32(camera, 0x24), (uint32_t)(int32_t)S16(camera, 0x28));
    COPY(0x10185c50, U32(camera, 0x3c));
    COPY(0x10185bd0, U32(camera, 0x2c));
    COPY(0x10185bf8, U32(camera, 0x40));
    COPY(0x10185bf0, U32(camera, 0x34));
    COPY(0x10185bec, U32(camera, 0x30));
    COPY(0x10185be0, U32(camera, 0x38));
    GF32(0x10185b60) = (float)(((double)GF32(0x10185bf8) - (double)GF32(0x10185c50)) / (double)GF32(0x1012bca4));
    COPY(0x10185bdc, U32(camera, 0x84));
    COPY(0x10185bd8, U32(camera, 0x88));
    COPY(0x10185bd4, U32(camera, 0x8c));
    COPY(0x10185bf4, U32(camera, 0x90));
    GF32(0x10185bfc) = (float)((double)GF32(0x10185be4) * (double)GF32(0x10185c20));
    COPY(0x10185ba0, G32(0x10185bfc));
    GF32(0x10185c00) = (float)((double)GF32(0x10185be4) * (double)GF32(0x10185c24));
    COPY(0x10185ba4, G32(0x10185c00));
    GF32(0x10185c04) = (float)((double)GF32(0x10185be4) * (double)GF32(0x10185c28));
    COPY(0x10185ba8, G32(0x10185c04));
    GF32(0x10185c08) = (float)((double)GF32(0x10185be8) * (double)GF32(0x10185c2c));
    COPY(0x10185bac, G32(0x10185c08));
    GF32(0x10185c0c) = (float)((double)GF32(0x10185be8) * (double)GF32(0x10185c30));
    COPY(0x10185bb0, G32(0x10185c0c));
    GF32(0x10185c10) = (float)((double)GF32(0x10185be8) * (double)GF32(0x10185c34));
    COPY(0x10185bb4, G32(0x10185c10));

    G32(0x10143fcc) = 0x3f800000u;
    if (GF32(0x10143fc4) < k || GF32(0x10143fc4) > k) {
        if (F32(camera, 4) > k) {
            if (GF32(0x10143fc4) > k)
                factor = (float)((double)GF32(0x10143fc4) / (double)F32(camera, 4));
            else
                factor = (float)(-(double)GF32(0x10143fc4) * (double)F32(camera, 4));
            if (!(factor >= GF32(0x101668cc)))
                COPY(0x10143fcc, G32(0x101668cc));
            else if (factor < GF32(0x1012bcac))
                GF32(0x10143fcc) = factor;
        } else if (GF32(0x10143fc4) < k) {
            COPY(0x10143fcc, G32(0x101668cc));
        }
    }
    if (G32(0x1017b940) != 0)
        GF32(0x10143fcc) = (float)((double)GF32(0x10143fcc) * (double)GF32(0x1012bcb0));
    else
        GF32(0x10143fcc) = (float)((double)GF32(0x10143fcc) * (double)GF32(0x1012bcb4));
}
