/*
 * Wave 2 (letter i) additions to unsorted_100a: two accessors around 0x100a9b00..0x100ab000.
 * Plain C, original cdecl convention. Names carry the original address
 * because the code alone does not say what the data is for.
 */
#ifndef HG_SIM_UNSORTED_100a_W2I_H
#define HG_SIM_UNSORTED_100a_W2I_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

uint32_t __cdecl Sim_100a9b7f(void *obj); /* the dword at +0x90 */
/*
 * Follows the dword at +8 of each node starting with the one at +4 of `node`;
 * returns the last non-NULL node, or NULL when node[4] is 0.
 */
void *__cdecl Sim_100aabfd(void *node);

#endif
