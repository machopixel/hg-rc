#include "w3o.h"
#include "../util/w3o.h"

static int Lerp(int base, int delta, float t)
{
    return (int)((uint32_t)base + (uint32_t)Render_Ftol((double)delta * t + GFL(0x1012ba54)));
}

// FUNCTION: HGSIM 0x100a2c08
void __cdecl Sim_100a2c08(int mode, int value, const int *a, const int *b, int *out)
{
    float t = 0.0f;

    if (mode == 1) {
        t = (float)((double)(int)((uint32_t)value - (uint32_t)a[0]) / (double)(int)((uint32_t)b[0] - (uint32_t)a[0]));
        out[0] = value;
        out[1] = Lerp(a[1], (int)((uint32_t)b[1] - (uint32_t)a[1]), t);
    } else if (mode == 2) {
        t = (float)((double)(int)((uint32_t)value - (uint32_t)a[1]) / (double)(int)((uint32_t)b[1] - (uint32_t)a[1]));
        out[0] = Lerp(a[0], (int)((uint32_t)b[0] - (uint32_t)a[0]), t);
        out[1] = value;
    }
    out[5] = Lerp(a[5], (int)((uint32_t)b[5] - (uint32_t)a[5]), t);
    out[2] = Lerp(a[2], (int)((uint32_t)b[2] - (uint32_t)a[2]), t);
    out[3] = Lerp(a[3], (int)((uint32_t)b[3] - (uint32_t)a[3]), t);
    out[4] = Lerp(a[4], (int)((uint32_t)b[4] - (uint32_t)a[4]), t);
}

// FUNCTION: HGSIM 0x100a78eb
int __cdecl Sim_100a78eb(uint32_t arg)
{
    return ((int(__cdecl *)(uint32_t))(uintptr_t)G32(0x1017d160))(arg);
}
