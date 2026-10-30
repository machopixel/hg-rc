#include "w3s.h"
#include "../render/w3s_fp.h"

/* One component of the transform: ((m0 * x + m1 * y) + m2 * z) + t, evaluated in double like the x87 unit. */
static float Dot3(const float *m, const uint8_t *v, double t)
{
    return (float)((((double)m[0] * (double)*(f32a *)(v + 0) + (double)m[1] * (double)*(f32a *)(v + 4)) +
                    (double)m[2] * (double)*(f32a *)(v + 8)) + t);
}

static float Dot3Only(const float *m, const uint8_t *v)
{
    return (float)(((double)m[0] * (double)*(f32a *)(v + 0) + (double)m[1] * (double)*(f32a *)(v + 4)) +
                   (double)m[2] * (double)*(f32a *)(v + 8));
}

// FUNCTION: HGSIM 0x10008eac
float __cdecl Sim_10008eac(const void *obj)
{
    const f32a *p = (const f32a *)(uintptr_t)U32(obj, 0x30);

    if (p == 0)
        return GF32(0x10111018);
    return *p;
}

// FUNCTION: HGSIM 0x100098bb
void __cdecl Sim_100098bb(void *block, const float *m)
{
    uint8_t *v = BYTES(block) + 0x18;
    uint8_t *n = BYTES(block) + U32(block, 8);
    int count = S16(block, 4);

    while (count != 0) {
        *(f32a *)(v + 0xc) = Dot3(m, v, (double)m[9]);
        *(f32a *)(v + 0x10) = Dot3(m + 3, v, (double)m[10]);
        *(f32a *)(v + 0x14) = Dot3(m + 6, v, (double)m[11]);
        v += 0x58;
        count--;
    }
    count = S16(block, 6);
    while (count != 0) {
        *(f32a *)(n + 0x18) = Dot3Only(m, n + 0xc);
        *(f32a *)(n + 0x1c) = Dot3Only(m + 3, n + 0xc);
        *(f32a *)(n + 0x20) = Dot3Only(m + 6, n + 0xc);
        n += 0x28;
        count--;
    }
}

// FUNCTION: HGSIM 0x10009a53
void __cdecl Sim_10009a53(void *obj, const float *m)
{
    U16(obj, 0) &= 0xfdff;
    F32(obj, 0x44) = Dot3(m, BYTES(obj) + 0x38, (double)m[9]);
    F32(obj, 0x48) = Dot3(m + 3, BYTES(obj) + 0x38, (double)m[10]);
    F32(obj, 0x4c) = Dot3(m + 6, BYTES(obj) + 0x38, (double)m[11]);
    F32(obj, 0x60) = Dot3(m, BYTES(obj) + 0x54, (double)m[9]);
    F32(obj, 0x64) = Dot3(m + 3, BYTES(obj) + 0x54, (double)m[10]);
    F32(obj, 0x68) = Dot3(m + 6, BYTES(obj) + 0x54, (double)m[11]);
    F32(obj, 0x7c) = Dot3(m, BYTES(obj) + 0x70, (double)m[9]);
    F32(obj, 0x80) = Dot3(m + 3, BYTES(obj) + 0x70, (double)m[10]);
    F32(obj, 0x84) = Dot3(m + 6, BYTES(obj) + 0x70, (double)m[11]);
    U32(obj, 0x9c) = U32(obj, 0x9c) + 1;
}

// FUNCTION: HGSIM 0x10009c55
void __cdecl Sim_10009c55(void *obj, const float *m)
{
    uint32_t child;

    Sim_10009a53(obj, m);
    for (child = U32(obj, 0x2c); child != 0; child = U32(child, 0xc)) {
        Sim_100098bb(BYTES(child), m);
        U32(child, 0x10) = U32(obj, 0x9c);
    }
}
