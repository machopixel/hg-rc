#include "w2a.h"
#include "../util/access.h"

#define TIMER_B GS32(0x101354d4)
#define TIMER_A1 GS32(0x101354d0)
#define TIMER_A2 GS32(0x101354d8)
#define TIMERS_READY GS32(0x1015dca0)

// FUNCTION: HGSIM 0x1002a8c2
void __cdecl Sim_1002a8c2(void)
{
    GS32(0x1015dc70) = 5;
    GS32(0x1015dc74) = 0;
    if (TIMERS_READY == 0) {
        Timer_Step();
        TIMER_B = (int16_t)Timer_Alloc(0x100); /* the original sign-extends the 16-bit number */
        TIMER_A1 = (int16_t)Timer_Alloc(0x80);
        TIMER_A2 = (int16_t)Timer_Alloc(0x80);
        Timer_Reset(TIMER_B);
        Timer_Reset(TIMER_A2);
        Timer_Reset(TIMER_A1);
        TIMERS_READY = 1;
    }
    GS32(0x1015dc98) = 0;
    GS32(0x1015dc78) = 0;
    GS32(0x1015dc7c) = 0;
}

// FUNCTION: HGSIM 0x1002ab26
void __cdecl Sim_1002ab26(void)
{
    if (TIMERS_READY != 0) {
        Timer_Free(TIMER_B);
        Timer_Free(TIMER_A2);
        Timer_Free(TIMER_A1);
        GS32(0x101354cc) = -1;
        TIMERS_READY = 0;
    }
}

// FUNCTION: HGSIM 0x1002ab9d
int __cdecl Sim_1002ab9d(void)
{
    return Timer_Elapsed(TIMER_A1);
}

// FUNCTION: HGSIM 0x1002abb0
int __cdecl Sim_1002abb0(void)
{
    return Timer_Elapsed(TIMER_A2);
}
