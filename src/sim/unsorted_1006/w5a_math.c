#include "w5a.h"
#include "../util/w3o.h"

// FUNCTION: HGSIM 0x10060882
void __cdecl Sim_10060882(float *x, float *y, float *z)
{
    double dx = *x;
    double dy = *y;
    double dz = *z;
    double inverse = GF64(0x1012b538) / W5A_SQRT(dx * dx + dy * dy + dz * dz);

    *x = (float)(inverse * dx);
    *y = (float)(inverse * dy);
    *z = (float)(inverse * dz);
}

// FUNCTION: HGSIM 0x100610ae
void __cdecl Sim_100610ae(float a, float b, float c, float ang, float *out)
{
    float half;
    float s;

    if (out == 0)
        W5A_ASSERT(SIM_PTR(char, 0x10144e54), SIM_PTR(char, 0x10144e34), 0x2a2);
    half = (float)((double)ang / (double)GFL(0x1012b550));
    s = (float)W5A_SINTAB(half);
    out[3] = (float)W5A_SINTAB((float)((double)half + (double)GFL(0x1012b55c)));
    out[0] = (float)((double)s * (double)a);
    out[1] = (float)((double)s * (double)b);
    out[2] = (float)((double)s * (double)c);
}

// FUNCTION: HGSIM 0x10068c48
void __cdecl Sim_10068c48(float a, uint32_t bits, float c, float ang, float *p1, uint32_t *p2, float *p3)
{
    float s;
    float k;
    /* The original negates a * k (fchs) before the add; kept as a separate store so the compiler cannot turn
     * -x + y into y - x, which gives the other sign for a NaN (e.g. a = inf, k = 0). */
    volatile double negAk;

    if (p1 == 0 || p2 == 0 || p3 == 0)
        return;
    if (!(W5A_FABS((double)ang) >= (double)GFL(0x1012b5f8)))
        return;
    s = (float)W5A_SINTAB((float)((double)ang + (double)GFL(0x1012b650)));
    k = (float)W5A_SINTAB(ang);
    *p1 = (float)((double)a * (double)s + (double)c * (double)k);
    *p2 = bits;
    negAk = -((double)a * (double)k);
    *p3 = (float)(negAk + (double)c * (double)s);
}

// FUNCTION: HGSIM 0x10069f68
void __cdecl Sim_10069f68(void *obj)
{
    double sum;
    float result;

    if (S32(obj, 0x50c) == 1)
        return;
    sum = (double)FL(U32(obj, 0), 0x68) + (double)FL(obj, 0xc);
    result = (float)W5A_FMOD(W5A_FMOD(sum, 360.0) + GF64(0x1012b668), 360.0);
    FL(U32(obj, 0), 0x110) = result;
}

// FUNCTION: HGSIM 0x1006ab98
void __cdecl Sim_1006ab98(uint32_t a, void *b, int i)
{
    uint32_t record = U32(b, 0x88) + (uint32_t)i * 0x3cu - 0x3cu;
    int count = S16(record, 0x34);
    int k;

    for (k = 0; k < count; k++)
        W5A_CALL_1006b52f(a, b, i, 0, 1);
}
