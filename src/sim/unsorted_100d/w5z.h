/*
 * Round 5 (letter z): small functions of HGSIM.DLL between 0x100d18e8 and 0x100d379f.
 * Calls of functions that are not converted yet go through their original
 * address (SIM_FN); the others call the converted C functions by name.
 */
#ifndef HG_SIM_UNSORTED_100D_W5Z_H
#define HG_SIM_UNSORTED_100D_W5Z_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Number of nodes of the tree below node, node included: the children are the pointers at +0xc and +8. 0 for NULL. */
uint32_t __cdecl Tree_CountNodes(void *node);

/* Calls the original 0x100d2306(a), then returns the original 0x100d255e(a). */
uint32_t __cdecl Sim_100d25ce(uint32_t a);

/*
 * When dword 0x1017e650 is not 0 and the signed dword 0x1017e65c is 0 or less: clears 0x1017e650, calls the original
 * 0x100d3427(dword 0x10152424) and copies dword 0x10152424 to 0x10152420.
 */
void __cdecl Sim_100d33ec(void);

/* Returns the original 0x100d3510(dword 0x1017e654 + a, b, c). */
uint32_t __cdecl Sim_100d36d6(uint32_t a, uint32_t b, uint32_t c);

/*
 * When dword 0x1017e660 is set: Sim_1008c6b6(record 0x1017e640), clears 0x1017e660, calls the original
 * 0x100bb2b0(dword 0x1017e664, dword 0x1014ced4), clears 0x1017e664 and calls Sim_100d33ec.
 */
void __cdecl Sim_100d379f(void);

#endif
