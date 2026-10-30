/*
 * Wave 2, letter j: leaf functions of HGSIM.DLL around 0x10100000..0x10110000.
 */
#ifndef HG_SIM_UNSORTED_1010_W2J_H
#define HG_SIM_UNSORTED_1010_W2J_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * Re-encodes a flag word: bit 0 -> 0x10, 2 -> 8, 3 -> 4, 4 -> 2, 5 -> 1, 1 -> 0x80000,
 * 12 -> 0x40000; bits 10-11 (value 1/2/3 of the pair) -> 0x100/0x200/0x300
 * (0 adds nothing); bits 8-9 == 0 -> 0x20000, == 2 -> 0x10000.
 */
uint32_t __cdecl Sim_10108710(uint32_t flags);
/* Returns 0. */
int __cdecl Sim_101091f0(void);

#endif
