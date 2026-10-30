#include "w5c.h"
#include "w2d.h"
#include "w3q.h"
#include "../unsorted_1009/w3o.h"
#include "../unsorted_1009/w3q.h"
#include "../util/util.h"
#include "../util/access.h"

typedef float HG_MAY_ALIAS w5c_f32;

// FUNCTION: HGSIM 0x100d6f16
void __cdecl Sim_100d6f16(void *obj, void *out)
{
    float a;
    float b;
    float c;
    float d;
    float e;
    float f;

    SIM_CALL_100d6f92(obj, &a, &b, &c);
    Sim_100a9cfb((void *)(uintptr_t)U32(obj, 0x4c), (uint32_t *)&d, (uint32_t *)&e, (uint32_t *)&f);
    if (G32(0x1015ef1c) != 0)
        e = (float)((double)e + (double)*(w5c_f32 *)(uintptr_t)G32(0x1015ef1c));
    Sim_1009cb40(out, d, e, f, a, b, c, 150000.0f);
}

// FUNCTION: HGSIM 0x100d6ea7
void __cdecl Sim_100d6ea7(void *obj, float *x, float *y, float *z, void *q)
{
    uint32_t block[17];
    float a;
    float b;
    float c;
    float angle;

    Sim_100d6f16(obj, block);
    angle = (float)Sim_100d6e99(obj);
    Sim_1009cd36(block, angle);
    Sim_100d709d(obj, (uint32_t *)&a, (uint32_t *)&b, (uint32_t *)&c, q);
    *x = (float)((double)*(w5c_f32 *)&block[3] - (double)a);
    *y = (float)((double)*(w5c_f32 *)&block[4] - (double)b);
    *z = (float)((double)*(w5c_f32 *)&block[5] - (double)c);
}
