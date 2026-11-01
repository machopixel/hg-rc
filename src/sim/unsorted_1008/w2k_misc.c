#include "w2k.h"
#include "../render/w33.h"
#include "../unsorted_1003/w2l.h"
#include "../util/access.h"

// FUNCTION: HGSIM 0x100877a5
void __cdecl Sim_100877a5(void)
{
    U32(G32(0x1015E698), 0) = 0;
}

/*
 * The start of display mode 6 (the renderer plugin, device class 3). 1.2 (0x10071b9c): in its 3DFX / PowerVR mode it
 * first sets the colour masks of the screen format record (1.2 0x107edf60 = 1.0 0x109e3880) to 5:6:5.
 */
// FUNCTION: HGSIM 0x100877bc
int __cdecl Sim_100877bc(void)
{
#ifndef W2KV_MUTATE /* the checker's negative check only: 1.0's empty start */
    if (g_w33Voodoo != 0)
        Sim_1003772b(SIM_PTR(uint32_t, 0x109e3880), 0xf800, 0x7e0, 0x1f, 0);
#endif
    return 0;
}

// FUNCTION: HGSIM 0x1008433f
int __cdecl Sim_1008433f(void)
{
    U32(G32(0x1015E698), 0) = G32(0x1015E9A0);
    return 0;
}

/* The product of two 16.16 numbers: high 32 bits of the 48-bit shift, plus the last bit shifted out. */
static int32_t FixedMul(int32_t a, int32_t b)
{
    int64_t product = (int64_t)a * b;

    return (int32_t)((uint32_t)(product >> 16) + (uint32_t)((product >> 15) & 1));
}

// FUNCTION: HGSIM 0x1008cd0a
int32_t *__cdecl Sim_1008cd0a(const int32_t *rect, const int32_t *m, int32_t *out)
{
    int32_t w = (int32_t)((uint32_t)rect[3] - (uint32_t)rect[1]);
    int32_t h = (int32_t)((uint32_t)rect[4] - (uint32_t)rect[2]);

    out[0] = FixedMul(w, m[0]);
    out[1] = FixedMul(h, m[1]);
    out[2] = FixedMul(w, m[2]);
    out[3] = FixedMul(h, m[3]);
    return out;
}
