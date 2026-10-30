/*
 * Round 18: game functions of HGSIM.DLL around 0x10020000..0x1002ffff that read unset stack (DECISIONS.md, "Round 18").
 */
#ifndef HG_SIM_UNSORTED_1002_W18_H
#define HG_SIM_UNSORTED_1002_W18_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * With [0x1015d52c] set, the original stores the volume 0 ([0x101321b8] * 0, shifted down 16 bits) into the dword
 * [0x1015d0a4] + 0x294 + 4 * index of a sound slot, for an index that is not negative, but the index is its local
 * [ebp-4], which nothing writes.
 *
 * Deviation (DECISIONS.md, round 18): nothing in HGSIM.DLL calls 0x10020055 or holds its address (or its thunk's), so
 * the leftover can never be seen; the C takes the index as negative and stores nothing.
 */
void __cdecl Sim_10020055(void);

#endif
