#include "w2l.h"
#include "../util/access.h"

// FUNCTION: HGSIM 0x10087a43
uint32_t __cdecl Sim_10087a43(void)
{
    return 0x50605;
}

// FUNCTION: HGSIM 0x10087b23
void __cdecl Sim_10087b23(void)
{
    G32(0x1017c8c4) = 1;
}

// FUNCTION: HGSIM 0x10087b41
void __cdecl Sim_10087b41(void)
{
    /* Dword values from 0x1017c840 up, in address order. */
    static const int32_t values[32] = {
        0, 0, -1, 0, 0, -1, -1, -1, 0, 0, -1, 0, 0, 1, -1, 1, 0, 0, 1, 0, 0, -1, 1, -1, 0, 0, 0, 1, 1, 0, 1, 1,
    };
    int i;

    for (i = 0; i < 32; i++)
        GS32(0x1017c840u + (uint32_t)i * 4) = values[i];
}
