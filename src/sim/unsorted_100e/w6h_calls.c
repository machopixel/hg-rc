#include "w6h.h"
#include "../util/w3t.h"

/* value, or 1.0f when its fabs is below the float at 0x1012bdc4 (or NaN); as raw bits. */
static uint32_t W6h_NotTiny(float value)
{
    union {
        float f;
        uint32_t u;
    } v;

    if (!(W6H_FABS_E((double)value) >= (double)GFL32(0x1012bdc4)))
        return 0x3f800000;
    v.f = value;
    return v.u;
}

// FUNCTION: HGSIM 0x100e2776
void __cdecl Sim_100e2776(float x, float y, float z)
{
    G32(0x1015264c) = W6h_NotTiny(x);
    G32(0x10152650) = W6h_NotTiny(y);
    G32(0x10152654) = W6h_NotTiny(z);
}
