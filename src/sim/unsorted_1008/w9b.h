/*
 * Round 9, letter b, unsorted_1008: the sweep over the scene lists that re-links every object of kind 0xc0.
 */
#ifndef HG_SIM_UNSORTED_1008_W9B_H
#define HG_SIM_UNSORTED_1008_W9B_H

#include <stdint.h>
#include "../simmem.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * For every object (next at +8) whose word +2 has 0xc0 in its mask 0xf0: with `all` set walks the list
 * [0x1017b9e0] calling Sim_1008a46e and Sim_1008a220; otherwise walks the 9 x 9 cell lists at 0x1017ba30
 * (0x18c per column, 0x2c per row) when [0x1017c8c4] is set, else the list [0x1017b990], calling Sim_1008a40a and
 * Sim_1008a37f. The next pointer is read before the calls.
 */
void __cdecl Sim_1008a94d(int all);

#endif
