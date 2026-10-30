/*
 * Round 4, letter v: a function of HGSIM.DLL around 0x10048000 that allocates a record.
 */
#ifndef HG_SIM_UNSORTED_1004_W4V_H
#define HG_SIM_UNSORTED_1004_W4V_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * Adds a record to the pointer table 0x109b94e0 (at index [0x1015f1c8], which is then incremented): a zero-filled
 * 0x1c-byte block with +0x10 = a, +8 = b, +0x14 = *d (sign-extended word). Returns the index, or 0xffff when the table
 * is full (7 entries), kind != 1, *d < 1 (then d | 0xffff) or the allocation fails. In the original the upper half
 * of eax on the 0xffff results is whatever the caller had in eax (here: 0).
 */
uint32_t __cdecl Sim_10048112(uint32_t a, uint32_t b, int32_t kind, const int16_t *d);

#endif
