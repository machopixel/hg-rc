#include "w2g.h"
#include "../simmem.h"

// FUNCTION: HGSIM 0x100cb9a4
int __cdecl Sim_100cb9a4(void)
{
    uint8_t *obj = (uint8_t *)(uintptr_t)SIM_PTR(uint32_t, 0x109E8B40)[*SIM_PTR(uint32_t, 0x1017B8C8)];
    uint32_t f = *(uint32_t *)(obj + 0xF0);

    if ((f & 0xF00) != 0x200)
        return -1;
    return (int)(f & 0xFF);
}
