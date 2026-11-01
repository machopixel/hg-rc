#ifndef HG_SIM_UNSORTED_1003_W36_H
#define HG_SIM_UNSORTED_1003_W36_H

#include <stdint.h>

/* Network round (2026-11-01): 1.2's network functions, converted from 1.2 (they were original code in 1.0's form). */

/* "DA" (a remote unit's state, 0x42 bytes): ignored once the game is over; the unit's position, speed, aim and
   state from the message (the position advanced by the smoothed lag unless that drops it 60 under the ground),
   the kill message and score when it is destroyed (state 4 / 5), the respawn when it comes back. 1.2 0x10032cbe. */
void __cdecl Sim_1003f3f8(const uint8_t *msg, int who, int size);

#endif
