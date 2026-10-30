#include "w3s.h"
#include "../render/w3s_fp.h"

// FUNCTION: HGSIM 0x100570fb
int __cdecl Sim_100570fb(int a, int b, int c, double d)
{
    int32_t v = (int32_t)((uint32_t)a - (uint32_t)c);
    int32_t negb = (int32_t)(0u - (uint32_t)b);

    if (v < 0) {
        if (v < negb) {
            v = (int32_t)((uint32_t)v + (uint32_t)b);
            v = Render_Ftol((double)v * d);
            if (v < -0x10000)
                v = -0x10000;
        } else {
            v = 0;
        }
    } else if (v > b) {
        v = (int32_t)((uint32_t)v - (uint32_t)b);
        v = Render_Ftol((double)v * d);
        if (v > 0x10000)
            v = 0x10000;
    } else {
        v = 0;
    }
    return v;
}

// FUNCTION: HGSIM 0x1005e295
void __cdecl Sim_1005e295(void *obj, int max, int val)
{
    if (val < 1)
        val = 1;
    else if (val > max)
        val = max;

    if (U32(obj, 0xbd3) & 0x10)
        val = (int)((uint32_t)val + (uint32_t)Render_Ftol((double)val * GD64(0x1012b4e0)));
    else if (U32(obj, 0xbd3) & 8)
        val = (int)((uint32_t)val + (uint32_t)Render_Ftol((double)val * GD64(0x1012b4e8)));
    else if (U32(obj, 0xbd3) & 4)
        val = (int)((uint32_t)val + (uint32_t)Render_Ftol((double)val * GD64(0x1012b4f0)));
    F32(obj, 0x4d4) = (float)((double)val * (double)GF32(0x1012b4d4));

    if (U32(obj, 0xbd3) & 0x20)
        S32(obj, 0x60b) = S32(obj, 0x60b) / 2;

    if (U32(obj, 0xbcf) & 8)
        U32(obj, 0xbd7) = U32(obj, 0xbd7) + 3;
    else if (U32(obj, 0xbcf) & 4)
        U32(obj, 0xbd7) = U32(obj, 0xbd7) + 2;
    else if (U32(obj, 0xbcf) & 2)
        U32(obj, 0xbd7) = U32(obj, 0xbd7) + 1;

    if (U32(obj, 0xbd3) & 0x40)
        U32(obj, 0xbe3) = U32(obj, 0xbe3) + 1;
    if (U32(obj, 0xbd3) & 0x80)
        U32(obj, 0xbdf) = U32(obj, 0xbdf) + 1;
}
