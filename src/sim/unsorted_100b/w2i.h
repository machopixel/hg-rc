/*
 * Wave 2 (letter i) additions to unsorted_100b: two small functions around 0x100b3500..0x100b4400.
 * Plain C, original cdecl convention. Names carry the original address
 * because the code alone does not say what the data is for.
 */
#ifndef HG_SIM_UNSORTED_100b_W2I_H
#define HG_SIM_UNSORTED_100b_W2I_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

#include "../unsorted_1003/w2i.h"

/* Returns Sim_1003a53b(); the argument is ignored. */
uint32_t __cdecl Sim_100b3520(uint32_t unused);
/*
 * Entry `index` of the table of 0x88-byte entries at 0x101a2100: clears bits
 * 12..15 of its dword at +0x18 and sets them to the low 4 bits of v.
 */
void __cdecl Sim_100b4300(int index, uint32_t v);

#endif
