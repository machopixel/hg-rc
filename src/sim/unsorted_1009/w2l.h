/*
 * Wave 2l: a doubly linked list with its ends in the globals 0x1017cc28 (first)
 * and 0x1017cc2c (last), and a pool of 0x57e-byte records at 0x10217c20 that is
 * taken from it, plus two getters. Names carry the original address because the
 * code alone does not say what the records are. Plain C, original cdecl
 * convention.
 */
#ifndef HG_SIM_UNSORTED_1009_W2L_H
#define HG_SIM_UNSORTED_1009_W2L_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * Unlinks node (prev dword +0, next dword +4) from the list whose first and last nodes are in 0x1017cc28 and
 * 0x1017cc2c, fixing those two when node was at an end, and clears its two links. Does nothing for a NULL node or
 * when either end is NULL.
 */
void __cdecl Sim_1009044e(void *node);
/*
 * Unless [0x10215ae8] is [0x1017cc20] - 1 or index is outside 0..[0x1017cc20] - 1: increments [0x10215ae8],
 * stores index in the dword array at 0x10216b80 under the new value, Sim_1009044e on the record at
 * 0x10217c20 + 0x57e * index and returns that record. Otherwise returns NULL.
 */
void *__cdecl Sim_10090513(int index);
/* For 0 < index < [0x1017cc20]: dword +0x57a of record `index` = 0, then Sim_10090513(index). */
void __cdecl Sim_10090cc8(int index);
/* Returns 0. */
uint32_t __cdecl Sim_1009ca21(void);

#endif
