#include "w2b.h"
#include "../simmem.h"

#define TIMER_D0 (*SIM_PTR(uint32_t, 0x101354D0))
#define TIMER_D4 (*SIM_PTR(uint32_t, 0x101354D4))
#define TIMER_D8 (*SIM_PTR(uint32_t, 0x101354D8))

// FUNCTION: HGSIM 0x1002abc3
void __cdecl Sim_1002abc3(void)
{
    Timer_Reset(TIMER_D8);
}

// FUNCTION: HGSIM 0x1002abd6
void __cdecl Sim_1002abd6(void)
{
    Timer_Reset(TIMER_D4);
    Timer_Reset(TIMER_D8);
    Timer_Reset(TIMER_D0);
    *SIM_PTR(uint32_t, 0x1015DC98) = 0;
    *SIM_PTR(uint32_t, 0x1015DC78) = *SIM_PTR(uint32_t, 0x1015DC98);
}

// FUNCTION: HGSIM 0x1002ac1b
int __cdecl Sim_1002ac1b(void)
{
    return Timer_Elapsed(TIMER_D4);
}
