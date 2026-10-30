#include "w2i.h"
#include "../util/access.h"

// FUNCTION: HGSIM 0x100c3702
void __cdecl Sim_100c3702(void *obj, int value)
{
    U8(U32(obj, 0x54), 0x21) = (uint8_t)value;
    U8(U32(obj, 0x54), 0x22) = (uint8_t)((int16_t)value == 0);
}

// FUNCTION: HGSIM 0x100c6f0e
int __cdecl Sim_100c6f0e(void *obj, int index)
{
    if (index < 0 || index >= S32(obj, 0x4b4))
        return -1;
    return S32(obj, (uint32_t)index * 12u + 0xcu);
}
