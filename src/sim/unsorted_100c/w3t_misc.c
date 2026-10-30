#include "w3t.h"
#include "../util/w3t.h"

// FUNCTION: HGSIM 0x100c3725
float __cdecl Sim_100c3725(void *obj)
{
    float rate;

    if (GS32(0x1015dc80) == 0)
        return 0.0f;
    rate = (float)(((double)FL32(obj, 0x268) - (double)FL32(obj, 0xd4)) / (double)GS32(0x1015dc80));
    U32(obj, 0x268) = U32(obj, 0xd4);
    return rate;
}
