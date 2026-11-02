#include "w2j.h"
#include "../util/access.h"

// FUNCTION: HGSIM 0x10098a49
uint32_t __cdecl Sim_10098a49(uint32_t unused)
{
    return U32(G32(0x1015d1b8), 0x20);
}

// FUNCTION: HGSIM 0x1009a6b3
int __cdecl Sim_1009a6b3(const void *obj, int bit)
{
    if ((S16(obj, 0x24) & 0x20) != 0 && ((1u << (bit & 31)) | (uint32_t)(int32_t)S16(obj, 0x26)) != 0)
        return 1;
    return 0;
}

// FUNCTION: HGSIM 0x1009b362
void __cdecl Sim_1009b362(void)
{
    if (GS32(0x1015d1ac) == 4 && GS32(0x1015d1cc) == 4)
        G32(0x1015d1cc) = 5;
    else if (GS32(0x1015d1ac) <= 2 && GS32(0x1015d1cc) <= 2)
        G32(0x1015d1cc) = 3;
}

// FUNCTION: HGSIM 0x1009b2fc
void __cdecl Sim_1009b2fc(void)
{
    if (GS32(0x1015d1ac) == 4)
        Sim_1009b362();
}
