#include "w5y.h"
#include "../util/access.h"

// FUNCTION: HGSIM 0x10098826
void __cdecl Sim_10098826(uint32_t unused, uint32_t value)
{
    (void)unused;
    SIM_FN(int(__cdecl *)(uint32_t), 0x10090d15)(value);
    U32(G32(0x1015d1b8), 0x28) = value;
}

// FUNCTION: HGSIM 0x100999bc
void __cdecl Sim_100999bc(void)
{
    G32(0x1015d104) = G32(0x1015d1c8);
    G32(0x1015d1c8) = 4;
    if (GS32(0x1015d1ac) <= 2)
        SIM_FN(void(__cdecl *)(void), 0x10099c36)();
    else if (GS32(0x1015d1ac) == 4 && GS32(0x1015d1cc) <= 2)
        SIM_FN(int(__cdecl *)(void), 0x100999fc)();
}
