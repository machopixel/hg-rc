#include "w3s.h"
#include "fns.h"
#include "w2e.h"
#include "../render/w3s_fp.h"
#include "../unsorted_1002/w3s.h"

// FUNCTION: HGSIM 0x10047b76
int __cdecl Sim_10047b76(void *rec, uint32_t a, uint32_t b, double d)
{
    U32(rec, 4) = a;
    U32(rec, 8) = b;
    F32(rec, 0xc) = (float)(d * GD64(0x101115d8));
    U32(rec, 0) = G32(0x1015dc78);
    return F32(rec, 0xc) > GF32(0x101115e0);
}

// FUNCTION: HGSIM 0x10047c92
int __cdecl Sim_10047c92(void *rec, uint32_t a, uint32_t b, double d, uint32_t e)
{
    U32(rec, 4) = a;
    U32(rec, 8) = b;
    F32(rec, 0xc) = (float)(d * GD64(0x101115d8));
    U32(rec, 0) = G32(0x1015dc78);
    U32(rec, 0x10) = e;
    return F32(rec, 0xc) > GF32(0x101115e4);
}

// FUNCTION: HGSIM 0x10044420
void __cdecl Sim_10044420(void)
{
    int i;
    int32_t selected;

    Sim_10047b76(GBYTES(0x109b9bb0), 0, 0, 0.5);
    Sim_10047b76(GBYTES(0x109b9a90), 0, 0, 0.5);
    Sim_10047b76(GBYTES(0x109b9af0), 0, 0, 0.7);
    Sim_10047b76(GBYTES(0x109b9ba0), 0, 0, 1.0);
    Sim_10047b76(GBYTES(0x109b9b90), 0, 0, 0.2);
    Sim_10047b76(GBYTES(0x109b9ae0), 0, 0, 0.2);
    Sim_10047c92(GBYTES(0x109b9ac0), 0, 0, 0.2, 0x43b40000u);
    Sim_10047c92(GBYTES(0x109b9aa0), 0, 0, 1.0, 0x43b40000u);
    G32(0x1015fdb4) = 0x3f800000u;
    for (i = 0; i < 5; i++) {
        uint8_t *rec = GBYTES(0x109b9b00) + (uint32_t)i * 0x1cu;

        U32(rec, 8) = 0;
        U32(rec, 4) = 0;
        U32(rec, 0) = 0;
        U32(rec, 0x14) = 0;
        U32(rec, 0x10) = 0;
        U32(rec, 0xc) = 0;
        U32(rec, 0x18) = 0;
    }
    selected = GS32(0x1017b8c8);
    if (PTRTAB(selected) != 0) {
        G32(0x1015ef14) = PTRTAB(selected);
        G32(0x1015ef18) = (uint32_t)GS32(0x1017b8c8);
    }
    Sim_10043f10();
    Sim_100448cd(GS32(0x1015ef00));
}

// FUNCTION: HGSIM 0x1004487c
float __cdecl Sim_1004487c(const float *p)
{
    float r = Sim_1002db2d(p[0], p[1], p[2]);

    return (float)((double)r + (double)GF32(0x1011157c));
}

// FUNCTION: HGSIM 0x1004f19d
float *__cdecl Sim_1004f19d(const void *obj, float *out)
{
    out[0] = (float)((double)F32(G32(0x1014cae0), 0) - (double)F32(obj, 0x58));
    out[1] = (float)((double)F32(G32(0x1014cae0), 4) - (double)F32(obj, 0x5c));
    out[2] = (float)((double)F32(G32(0x1014cae0), 8) - (double)F32(obj, 0x60));
    return out;
}
