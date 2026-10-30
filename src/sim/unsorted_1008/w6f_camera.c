#include "w6f.h"
#include "../util/access.h"
#include "../util/w3o.h"
#include "../unsorted_100d/w5c.h"
#include "../unsorted_1003/w2b.h"

#define ADDR(p) ((uint32_t)(uintptr_t)(p))
#define CAMERA BYTES(G32(0x1014cae0))

/* The original copies with `rep movsd`: dword by dword, upwards. */
static void CopyDwords(void *dst, const void *src, int count)
{
    u32a *d = (u32a *)dst;
    const u32a *s = (const u32a *)src;
    int i;

    for (i = 0; i < count; i++)
        d[i] = s[i];
}

// FUNCTION: HGSIM 0x1008fb20
void __cdecl Sim_1008fb20(const uint32_t *pos, int32_t preset, int32_t scale, int32_t depth)
{
    const uint8_t *rec = SIM_PTR(uint8_t, 0x10185400 + (uint32_t)preset * 0x14u);
    int32_t w = (int32_t)(U32(rec, 0xc) - U32(rec, 4) + 1u);
    int32_t h;
    double t;

    GFL(0x1036f1d8) = (float)((double)scale / (double)w);
    t = -(double)scale / (double)GFL(0x1012b938);
    GFL(0x1036f1c0) = (float)t;
    GFL(0x1036f1c4) = (float)-t;
    h = (int32_t)(U32(rec, 0x10) - U32(rec, 8) + 1u);
    t = (double)h * (double)GFL(0x1036f1d8) / (double)GFL(0x1012b938);
    GFL(0x1036f1c8) = (float)t;
    GFL(0x1036f1cc) = (float)-t;
    G32(0x1036f1d0) = 0;
    GFL(0x1036f1d4) = (float)depth;
    G32(0x1036f078) = G32(0x1017e650);
    CopyDwords(SIM_PTR(uint8_t, 0x1036f0e0), CAMERA, 0x38);
    U32(CAMERA, 0xc) = U32(pos, 0xc);
    U32(CAMERA, 0x10) = U32(pos, 0x10);
    U32(CAMERA, 0x14) = U32(pos, 0x14);
    U32(CAMERA, 0) = U32(pos, 0);
    U32(CAMERA, 4) = U32(pos, 4);
    U32(CAMERA, 8) = U32(pos, 8);
    Sim_100d32e5(preset);
    U32(CAMERA, 0x40) = G32(0x1036f1d4);
    CopyDwords(SIM_PTR(uint8_t, 0x1036f1e0), SIM_PTR(uint8_t, 0x1014cae8), 0x1a);
    G32(0x1014cb40) = ADDR(SIM_PTR(uint8_t, 0x10001f78));
    G32(0x1014cb44) = ADDR(SIM_PTR(uint8_t, 0x1000239c));
    G32(0x1036f0cc) = G32(0x1017d14c);
    G32(0x1017d14c) = 1;
    W6F_CALL_100d2306(CAMERA);
    U32(CAMERA, 0x94) = 0x3f800000u;        /* 1.0f */
    U32(CAMERA, 0x98) = U32(CAMERA, 0x44);
    Sim_100d255e(CAMERA);
    W6F_CALL_100d1f90(CAMERA);
    G32(0x1013a014) = 0;
}

// FUNCTION: HGSIM 0x1008fd5a
void __cdecl Sim_1008fd5a(void)
{
    CopyDwords(SIM_PTR(uint8_t, 0x1014cae8), SIM_PTR(uint8_t, 0x1036f1e0), 0x1a);
    CopyDwords(CAMERA, SIM_PTR(uint8_t, 0x1036f0e0), 0x38);
    G32(0x1017e650) = G32(0x1036f078);
    Sim_10035469();
    G32(0x1017d14c) = G32(0x1036f0cc);
    W6F_CALL_100d2306(CAMERA);
    Sim_100d255e(CAMERA);
    W6F_CALL_100d1f90(CAMERA);
    G32(0x1013a014) = 0;
}
