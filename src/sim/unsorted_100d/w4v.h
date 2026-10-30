/*
 * Round 4, letter v: two functions of HGSIM.DLL around 0x100da000 that allocate and free the heap blocks of the
 * default look-up tables.
 */
#ifndef HG_SIM_UNSORTED_100D_W4V_H
#define HG_SIM_UNSORTED_100D_W4V_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * Allocates (not zeroed, then cleared) the 0x60-byte block [0x1017e6bc] and the 0x48-byte block [0x1017e6c0], links the
 * second to the first (+0x44), clears +0x40 of the first and names them "DEFAULT_MLUT" (first, at +0) and "DEFAULT_TLUT"
 * (second, at +0x20).
 */
void __cdecl Sim_100da45c(void);
/* Frees the blocks [0x1017e688], [0x1017ee40 + 4*i] (i < 16) and [0x1017e68c] when not NULL, clearing each pointer. */
void __cdecl Sim_100db351(void);

#endif
