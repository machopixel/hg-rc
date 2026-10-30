/*
 * Converted functions of HGSIM.DLL's simulation core. Plain C with no
 * knowledge of the hook layer: src/hooks/hooks_core.c puts each one in place
 * of the original. Every function keeps the original calling convention.
 */
#ifndef HG_SIM_CORE_H
#define HG_SIM_CORE_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Data of the original DLL that the converted code reads and writes. */
typedef struct CoreImports {
    uint64_t *lastTick;  /* clock reading at the last whole step, 0 before the first call */
    uint64_t *stepLength; /* milliseconds per simulation step (5 = 200 steps a second) */
    void(__cdecl *step)(void); /* runs one simulation step */
    uint32_t(__cdecl *clock)(void); /* milliseconds, timeGetTime */
} CoreImports;

void Core_SetImports(const CoreImports *imports);

/*
 * Catches the simulation up with the clock: runs one step for every whole
 * step length that has passed since the last call and keeps the remainder, so
 * the simulation always runs at the same fixed rate whatever the frame rate.
 */
void Core_RunSteps(void);

/*
 * The timer module (original 0x100e4550..0x100e470f). Two counters tick once per
 * simulation step: counter A for timer numbers with bit 0x80 set, counter B for
 * the others. A timer is a slot in a table that remembers the counter value at
 * its start (0 = slot free, -1 = end of the table). Pause bits in the flag word:
 * 0x200 stops counter A, 0x100 stops counter B.
 */
void __cdecl Timer_Step(void);
/* Starts a timer; returns its number, or 0xFFFF when the table is full. */
int __cdecl Timer_Alloc(unsigned which);
int __cdecl Timer_Elapsed(unsigned timer);
void __cdecl Timer_Reset(unsigned timer);
/* Makes the timer read as if it had been started `ago` steps back. */
void __cdecl Timer_SetElapsed(unsigned timer, int ago);
void __cdecl Timer_Free(unsigned timer);
/* which: 0x80 = counter A, 0x100 = counter B; on non-zero pauses, zero resumes. */
void __cdecl Timer_Pause(unsigned short which, short on);

#endif
