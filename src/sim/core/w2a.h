/*
 * Wave 2a: users of the timer module (core.h). They keep the numbers of three
 * timers in globals of the DLL ([0x101354d0], [0x101354d4], [0x101354d8]); the
 * code alone does not say what the timers are for, so the names carry the
 * original address.
 */
#ifndef HG_SIM_CORE_W2A_H
#define HG_SIM_CORE_W2A_H

#include "core.h"

/*
 * Sets [0x1015dc70] = 5 and [0x1015dc74] = 0. When [0x1015dca0] is 0: steps the
 * counters once, allocates three timers (one of counter B, two of counter A)
 * into the three globals, resets all three and sets [0x1015dca0] = 1. Then
 * clears [0x1015dc98], [0x1015dc78] and [0x1015dc7c].
 */
void __cdecl Sim_1002a8c2(void);
/* When [0x1015dca0] is set: frees the three timers, sets [0x101354cc] = -1 and clears [0x1015dca0]. */
void __cdecl Sim_1002ab26(void);
/* Timer_Elapsed of the number in [0x101354d0]. */
int __cdecl Sim_1002ab9d(void);
/* Timer_Elapsed of the number in [0x101354d8]. */
int __cdecl Sim_1002abb0(void);

#endif
