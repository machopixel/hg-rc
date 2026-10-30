#include "w5c.h"
#include "w3o.h"
#include "../util/access.h"

typedef float HG_MAY_ALIAS w5c_f32;

// FUNCTION: HGSIM 0x1009cf67
void __cdecl Sim_1009cf67(void *obj, float t)
{
    float step;

    if (!(SIM_FABS_C((double)*(w5c_f32 *)(BYTES(obj) + 0x1c)) >= (double)*SIM_PTR(w5c_f32, 0x1012ba08)))
        return;
    step = (float)(((double)t - (double)*(w5c_f32 *)(BYTES(obj) + 4)) * (double)*(w5c_f32 *)(BYTES(obj) + 0x30)
                   / (double)*(w5c_f32 *)(BYTES(obj) + 0x1c));
    Sim_1009cd36(obj, step);
}
