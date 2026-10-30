#include "w5b.h"
#include "../util/access.h"

typedef float HG_MAY_ALIAS w5b_f32;
#define W5B_F32(p, off) (*(w5b_f32 *)(BYTES(p) + (off)))

// FUNCTION: HGSIM 0x10047e1a
float __cdecl Sim_10047e1a(void *obj, float x)
{
    float half = (float)((double)W5B_F32(obj, 0x10) / (double)*SIM_PTR(w5b_f32, 0x101115e8));
    double remainder = SIM_FMOD((double)x, (double)W5B_F32(obj, 0x10));
    float result = (float)remainder;

    if (remainder > (double)half)
        result = (float)((double)result - (double)W5B_F32(obj, 0x10));
    else if (-(double)half > (double)result)
        result = (float)((double)result + (double)W5B_F32(obj, 0x10));
    W5B_F32(obj, 4) = result;
    return result;
}

// FUNCTION: HGSIM 0x1004e3d7
void __cdecl Sim_1004e3d7(void)
{
    if (G32(0x1015f35c) != 0) {
        SIM_FREE(BYTES(G32(0x1015f35c)));
        G32(0x1015f35c) = 0;
    }
    if (G32(0x1015f358) != 0) {
        SIM_FREE(BYTES(G32(0x1015f358)));
        G32(0x1015f358) = 0;
    }
    if (G32(0x1015f354) != 0) {
        SIM_FREE(BYTES(G32(0x1015f354)));
        G32(0x1015f354) = 0;
    }
}

// FUNCTION: HGSIM 0x1004e441
int __cdecl Sim_1004e441(void)
{
    Sim_1004e3d7();
    G32(0x1015f354) = (uint32_t)(uintptr_t)SIM_MALLOC(0x1000);
    if (G32(0x1015f354) == 0) {
        Sim_1004e3d7();
        return 3;
    }
    G32(0x1015f358) = (uint32_t)(uintptr_t)SIM_MALLOC(0x4000);
    if (G32(0x1015f358) == 0) {
        Sim_1004e3d7();
        return 3;
    }
    G32(0x1015f35c) = (uint32_t)(uintptr_t)SIM_MALLOC(0x9bd4);
    if (G32(0x1015f35c) == 0) {
        Sim_1004e3d7();
        return 3;
    }
    return 0;
}
