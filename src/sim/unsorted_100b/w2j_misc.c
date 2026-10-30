#include "w2j.h"
#include "../util/access.h"

// FUNCTION: HGSIM 0x100b487e
int __cdecl Sim_100b487e(void)
{
    int found = 0;
    int i;

    for (i = 0; i < GS32(0x1017d218) && found == 0; i++) {
        if ((int8_t)GBYTES(0x101a2100)[i * 0x88] == 0)
            found = 1;
    }
    return found ? 0 : 1;
}

// FUNCTION: HGSIM 0x100b53bb
uint32_t __cdecl Sim_100b53bb(int index)
{
    uint32_t result = 0;

    if (index < GS32(0x1017d218) && index >= 0)
        result = U32(SIM_PTR(uint8_t, 0x101a2128u + (uint32_t)index * 0x88u), 0);
    return result;
}

// FUNCTION: HGSIM 0x100bda0b
int __cdecl Sim_100bda0b(const void *obj, uint32_t value)
{
    int i;

    for (i = 0; i < S32(obj, 0); i++) {
        if (U32(U32(obj, 8), (uint32_t)i * 4u) == value)
            return i;
    }
    return -1;
}
