#include "w5c.h"
#include "../util/access.h"

typedef float HG_MAY_ALIAS w5c_f32;
#define W5C_GF32(addr) (*SIM_PTR(w5c_f32, addr))

// FUNCTION: HGSIM 0x10068ce0
void __cdecl Sim_10068ce0(float x, float y, uint32_t raw, float angle, float *outA, float *outB, uint32_t *outC)
{
    float c;
    float s;

    if (outA == 0 || outB == 0 || outC == 0)
        return;
    if (!(SIM_FABS_C((double)angle) >= (double)W5C_GF32(0x1012b5f8)))
        return;
    c = (float)SIM_CALL_10084766_C((float)((double)angle + (double)W5C_GF32(0x1012b650)));
    s = (float)SIM_CALL_10084766_C(angle);
    *outA = (float)((double)x * (double)c - (double)y * (double)s);
    *outB = (float)((double)x * (double)s + (double)y * (double)c);
    *outC = raw;
}
