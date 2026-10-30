#include "w3t.h"
#include "../util/w3t.h"

// FUNCTION: HGSIM 0x100a9d7c
void __cdecl Sim_100a9d7c(void *obj, float dx, float dy, float dz)
{
    FL32(obj, 0x30) = (float)((double)FL32(obj, 0x30) + (double)dx);
    FL32(obj, 0x34) = (float)((double)FL32(obj, 0x34) + (double)dy);
    FL32(obj, 0x38) = (float)((double)FL32(obj, 0x38) + (double)dz);
    U32(obj, 0x94) = U32(obj, 0x94) | 1u;
}

// FUNCTION: HGSIM 0x100ab870
void __cdecl Camera_SetAspect(const int32_t *size)
{
    GFL32(0x101c64e0) = (float)((double)size[1] * (double)1.33333337f / (double)size[0]);
    U32(G32(0x1014cae0), 0x44) = G32(0x101c64e0);
}
