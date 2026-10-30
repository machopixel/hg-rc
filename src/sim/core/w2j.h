/*
 * Wave 2, letter j: users of the timer module (core.h).
 */
#ifndef HG_SIM_CORE_W2J_H
#define HG_SIM_CORE_W2J_H

#include "core.h"

/*
 * Per-frame update of the game clock. Does nothing while dword 0x1015dca0 is 0.
 * Mode dword 0x1015dc84 = 0 reads the time (dword 0x1015dc78) from the timer in
 * dword 0x101354d0, first waiting until it is at least dword 0x1015dc94 past the
 * previous time (dword 0x1015dc98) when that is positive; a non-zero dword
 * 0x1015dc88 then switches to mode 3 (saving the old mode in 0x1015dc9c) and
 * pauses counter A. In mode 3 the time advances by 12 per call until
 * 0x1015dc88 is 0, which restores the mode, resumes counter A and sets the
 * timer to the time. The step (dword 0x1015dc80) is time - previous time; unless
 * dword 0x1015ec58 is set it is multiplied by 8 when dword 0x1015dc8c is set,
 * else divided by 4 (arithmetic shift) when dword 0x1015dc90 is set, the time
 * advances by the changed step and the timer is set to it. A step <= 0 becomes
 * 0 and the time goes back to the previous time, which then takes the time.
 */
void __cdecl Sim_1002a983(void);

#endif
