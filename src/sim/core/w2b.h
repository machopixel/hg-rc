/*
 * Wave 2b: small users of the timer module (core.h). Each keeps the number of
 * a timer in a global of the DLL; the code alone does not say what the timers
 * are for, so the names carry the original address.
 */
#ifndef HG_SIM_CORE_W2B_H
#define HG_SIM_CORE_W2B_H

#include "core.h"

/* Timer_Reset on the timer number in [0x101354d8]. */
void __cdecl Sim_1002abc3(void);
/* Timer_Reset on the numbers in [0x101354d4], [0x101354d8] and [0x101354d0]; clears [0x1015dc98] and [0x1015dc78]. */
void __cdecl Sim_1002abd6(void);
/* Timer_Elapsed of the number in [0x101354d4]. */
int __cdecl Sim_1002ac1b(void);

#endif
