#include "core.h"
#include "../simmem.h"

/* Data of the original DLL (addresses at its preferred base). */
#define TIMER_FLAGS ((uint32_t *)SIM_PTR(uint32_t, 0x1015317C))
#define COUNTER_A SIM_PTR(uint32_t, 0x10153380)
#define COUNTER_B SIM_PTR(uint32_t, 0x10153384)
#define SLOTS_A SIM_PTR(uint32_t, 0x10153180)
#define SLOTS_B SIM_PTR(uint32_t, 0x10153280)

/* The slot of a timer number, and the counter its table belongs to. */
static uint32_t *Slot(unsigned timer, uint32_t **counter)
{
    if (timer & 0x80) {
        *counter = COUNTER_A;
        return SLOTS_A + (timer ^ 0x80);
    }
    *counter = COUNTER_B;
    return SLOTS_B + timer;
}

// FUNCTION: HGSIM 0x100e4550
void __cdecl Timer_Step(void)
{
    if ((*TIMER_FLAGS & 0x200) == 0)
        (*COUNTER_A)++;
    if ((*TIMER_FLAGS & 0x100) == 0)
        (*COUNTER_B)++;
}

// FUNCTION: HGSIM 0x100e4579
int __cdecl Timer_Alloc(unsigned which)
{
    uint32_t *counter = (which & 0x80) ? COUNTER_A : COUNTER_B;
    uint32_t *slot = (which & 0x80) ? SLOTS_A : SLOTS_B;
    unsigned number = 0;

    for (; *slot != 0; slot++) {
        if (*slot == 0xFFFFFFFFu)
            return (int)((which & 0xFFFF0000u) | 0xFFFF);
        number++;
    }
    *slot = *counter != 0 ? *counter : 1; /* 0 would read as a free slot */
    return (int)(which & 0x80 ? number | 0x80 : number);
}

// FUNCTION: HGSIM 0x100e45e7
int __cdecl Timer_Elapsed(unsigned timer)
{
    uint32_t *counter;
    uint32_t *slot = Slot(timer, &counter);

    return (int)(*counter - *slot);
}

// FUNCTION: HGSIM 0x100e4622
void __cdecl Timer_Reset(unsigned timer)
{
    uint32_t *counter;
    uint32_t *slot = Slot(timer, &counter);

    *slot = *counter;
}

// FUNCTION: HGSIM 0x100e465d
void __cdecl Timer_SetElapsed(unsigned timer, int ago)
{
    uint32_t *counter;
    uint32_t *slot = Slot(timer, &counter);

    *slot = *counter - (uint32_t)ago;
}

// FUNCTION: HGSIM 0x100e469b
void __cdecl Timer_Free(unsigned timer)
{
    uint32_t *counter;

    *Slot(timer, &counter) = 0;
}

// FUNCTION: HGSIM 0x100e46d0
void __cdecl Timer_Pause(unsigned short which, short on)
{
    uint32_t bits = 0;

    if (which & 0x80)
        bits |= 0x200;
    if (which & 0x100)
        bits |= 0x100;
    if (on == 0)
        *TIMER_FLAGS &= ~bits;
    else
        *TIMER_FLAGS |= bits;
}
