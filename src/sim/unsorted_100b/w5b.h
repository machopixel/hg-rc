/*
 * Round 5, letter b: a function around 0x100b77b3 that calls a C library routine of the DLL (free 0x100fefe0)
 * through its address with SIM_FN.
 */
#ifndef HG_SIM_UNSORTED_100B_W5B_H
#define HG_SIM_UNSORTED_100B_W5B_H

#include <stdint.h>
#include "../simmem.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Not converted: free(). */
#define SIM_FREE SIM_FN(void(__cdecl *)(void *), 0x100fefe0)

/*
 * Frees a counted array of pairs: block = { int count; entries *; }, entries = count pairs of two pointers (8 bytes
 * each). Frees both pointers of each pair, then the array, then the block (each only when block, and for the pairs
 * block[4], is not NULL).
 */
void __cdecl Sim_100b77b3(void *block);

#endif
