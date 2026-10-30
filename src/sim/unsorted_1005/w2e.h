/*
 * Wave 2e, original 0x100530a9 and 0x1005aa45.
 */
#ifndef HG_SIM_UNSORTED_1005_W2E_H
#define HG_SIM_UNSORTED_1005_W2E_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * Walks the list whose first node is [0x1015f3b0] (next pointer at +0). A node whose short +4 is
 * -2 or -1 matches when the string at node+6 equals the string at key+2; any other node matches
 * when its short +4 equals the short at key+0. Returns the address of node+4 of the first match, or 0.
 */
void *__cdecl Sim_100530a9(const void *key);
/*
 * Appends a record to the two parallel arrays at [0x1017d1f8] and [0x1017d1f4] (index
 * [0x101c42a0], which is incremented): the first receives Sim_100db770(a1), the second
 * the result of Sim_1003aa7d(a0, ...) with the keys translated by Sim_100b3839; then
 * Sim_1003b50f stores the low 16 bits of a4 in that new entry. a3 is range-checked (0..8,
 * else 4) but never used.
 */
void __cdecl Sim_1005aa45(const char *a0, int a1, int a2, int a3, uint32_t a4, uint32_t a5,
                          const void *a6, uint32_t a7);

#endif
