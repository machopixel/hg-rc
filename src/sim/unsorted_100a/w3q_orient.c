#include "w3q.h"
#include "../unsorted_1006/w3q.h"
#include "../util/access.h"

// FUNCTION: HGSIM 0x100aa1de
void __cdecl Sim_100aa1de(void *obj, const float *q)
{
    float *rot = (float *)(BYTES(obj) + 0x3c);

    Quat_Multiply(q, rot, rot);
    U32(obj, 0x94) = U32(obj, 0x94) & 0xffffbfffu;
    U32(obj, 0x94) = U32(obj, 0x94) | 1;
}

// FUNCTION: HGSIM 0x100aa227
void __cdecl Sim_100aa227(void *obj, const float *q)
{
    float *rot = (float *)(BYTES(obj) + 0x3c);

    Quat_Multiply(rot, q, rot);
    U32(obj, 0x94) = U32(obj, 0x94) & 0xffffbfffu;
    U32(obj, 0x94) = U32(obj, 0x94) | 1;
}

// FUNCTION: HGSIM 0x100aa2be
void __cdecl Sim_100aa2be(void *obj, const float *q)
{
    float *rot = (float *)(BYTES(obj) + 0x7c);

    Quat_Multiply(q, rot, rot);
    U32(obj, 0x94) = U32(obj, 0x94) & 0xffff7fffu;
    U32(obj, 0x94) = U32(obj, 0x94) | 1;
}
