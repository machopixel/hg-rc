#include "w3r.h"
#include "core.h"
#include "../simmem.h"

// FUNCTION: HGSIM 0x1002ab74
void __cdecl Sim_1002ab74(void)
{
    Timer_Reset(*SIM_PTR(uint32_t, 0x101354dc));
    while (Timer_Elapsed(*SIM_PTR(uint32_t, 0x101354dc)) < 0x5b)
        ;
}
