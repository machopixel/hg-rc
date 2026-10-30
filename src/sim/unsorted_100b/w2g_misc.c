#include "w2g.h"
#include "../simmem.h"

#define G32(addr) (*SIM_PTR(uint32_t, addr))

// FUNCTION: HGSIM 0x100b3547
void __cdecl Sim_100b3547(void)
{
    G32(0x10150634) = G32(0x101C47BC);
    G32(0x1015064C) = G32(0x101C45E4);
    G32(0x10150660) = G32(0x101C4858);
    G32(0x10150674) = G32(0x101C4F90);
    G32(0x10150688) = G32(0x101C4758);
    G32(0x1015069C) = G32(0x101C4FD8);
}

// FUNCTION: HGSIM 0x100b367e
int __cdecl Sim_100b367e(int a, int b)
{
    int result = 0;

    if ((int32_t)G32(0x1017D20C) < (int32_t)G32(0x1017D210)) {
        uint32_t *entry = (uint32_t *)(uintptr_t)(G32(0x1017D214) + G32(0x1017D20C) * 8u);

        entry[0] = (uint32_t)a;
        entry[1] = (uint32_t)b;
        G32(0x1017D20C) = G32(0x1017D20C) + 1;
        result = 1;
    }
    return result;
}

// FUNCTION: HGSIM 0x100b6590
int __cdecl Sim_100b6590(void)
{
    return 1;
}

// FUNCTION: HGSIM 0x100b65b2
void __cdecl Sim_100b65b2(void)
{
}
