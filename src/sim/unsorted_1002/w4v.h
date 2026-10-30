/*
 * Round 4, letter v: a function of HGSIM.DLL around 0x1002a000 that allocates through Heap_AllocRaw.
 */
#ifndef HG_SIM_UNSORTED_1002_W4V_H
#define HG_SIM_UNSORTED_1002_W4V_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * Allocates (not zeroed) 0x2a0 bytes, or 0x2fc when index is not the player's gear ([0x1017b8c8]), and stores the
 * block in the pointer table 0x109e8b40 at `index`. Returns 1, or 0 when the allocation failed (table untouched).
 */
int __cdecl Sim_1002a29f(int index);

#endif
