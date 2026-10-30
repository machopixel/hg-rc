#include "w3q.h"
#include "../render/w2n.h"
#include "../unsorted_100b/fns.h"
#include "../util/w3q.h"

// FUNCTION: HGSIM 0x10098be0
void __cdecl Sim_10098be0(void)
{
    int i;
    int found = 0;
    uint32_t a, b, c;
    uint8_t *obj;
    float k;

    for (i = 0; i < 16; i++) {
        uint8_t *rec = GBYTES(0x101d0440) + (uint32_t)i * 0x40;

        G32(0x101d0980 + (uint32_t)i * 4) = 0xffffffffu;
        G32(0x101d0940 + (uint32_t)i * 4) = (uint32_t)i;
        U32(rec, 0) = (uint32_t)i;
        U32(rec, 0x18) = 0;
        U32(rec, 0x30) = 0;
        U32(rec, 0x2c) = 0;
        U32(rec, 0x28) = 0;
        U32(rec, 0x24) = 0;
        U32(rec, 0x20) = 0;
        U32(rec, 0x1c) = 0;
        U32(rec, 0x3c) = 0xffffffffu;
        U32(rec, 0xc) = 0;
        U32(rec, 8) = 0;
        U32(rec, 0x38) = 0;
        U32(rec, 0x34) = 0;
        U32(rec, 4) = 0;
        U32(rec, 0x10) = 0xffffffffu;
    }
    for (i = 0; i < GS32(0x1015dc68) && !found; i++) {
        if ((G32(0x109e9bac + (uint32_t)i * 0x50) & 1) == 0) {
            found = 1;
            G32(0x1014bd8c) = (uint32_t)i;
        }
    }
    G32(0x1014bd90) = G32(0x109e9ba8 + (uint32_t)i * 0x50);
    Sim_100b53f0((int)G32(0x1014bd90), &a, &b, &c);
    obj = BYTES(G32(0x1014bd88));
    k = GF32(0x1012b9ac);
    U32(obj, 0x18) = 1;
    {
        union {
            uint32_t u;
            float f;
        } fa, fb, fc;

        fa.u = a;
        fb.u = b;
        fc.u = c;
        F32A(obj, 0x1c) = (float)((double)fa.f - (double)k);
        F32A(obj, 0x20) = (float)((double)fb.f - (double)k);
        F32A(obj, 0x24) = (float)((double)fc.f - (double)k);
        F32A(obj, 0x28) = (float)((double)fa.f + (double)k);
        F32A(obj, 0x2c) = (float)((double)fb.f + (double)k);
        F32A(obj, 0x30) = (float)((double)fc.f + (double)k);
    }
    U32(obj, 0x34) = 0;
    U32(obj, 0x38) = 0;
    U32(obj, 0x3c) = 0;
    U32(obj, 0x10) = 0xffffffffu;
}

// FUNCTION: HGSIM 0x1009b876
int __cdecl Sim_1009b876(const void *obj, float f)
{
    int32_t d = (int32_t)(U32(obj, 0x18) - (uint32_t)Render_Ftol((double)f));
    double t;
    int n;

    d = (int32_t)((uint32_t)d - G32(0x1017cc58));
    t = (double)d / (double)GS32(0x1014bf00);
    n = Render_Ftol(t * (double)GF32(0x1012b9d8));
    if (n < 0)
        n = 0;
    else if (n > 0xf)
        n = 0xf;
    return n;
}

// FUNCTION: HGSIM 0x1009cb40
void __cdecl Sim_1009cb40(void *obj, float a1, float a2, float a3, float a4, float a5, float a6, float scale)
{
    F32A(obj, 0) = a1;
    F32A(obj, 4) = a2;
    F32A(obj, 8) = a3;
    F32A(obj, 0x24) = a4;
    F32A(obj, 0x28) = a5;
    F32A(obj, 0x2c) = a6;
    F32A(obj, 0x18) = (float)((double)scale * (double)a4);
    F32A(obj, 0x1c) = (float)((double)scale * (double)a5);
    F32A(obj, 0x20) = (float)((double)scale * (double)a6);
    F32A(obj, 0xc) = (float)((double)a1 + (double)F32A(obj, 0x18));
    F32A(obj, 0x10) = (float)((double)a2 + (double)F32A(obj, 0x1c));
    F32A(obj, 0x14) = (float)((double)a3 + (double)F32A(obj, 0x20));
    F32A(obj, 0x30) = scale;
    U32(obj, 0x34) = 1;
}
