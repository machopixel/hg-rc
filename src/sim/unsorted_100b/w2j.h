/*
 * Wave 2, letter j: leaf functions of HGSIM.DLL around 0x100b0000..0x100c0000.
 */
#ifndef HG_SIM_UNSORTED_100B_W2J_H
#define HG_SIM_UNSORTED_100B_W2J_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * 1 when none of the first `dword 0x1017d218` entries of the 0x88-byte table at
 * 0x101a2100 has a zero first byte (signed byte), else 0.
 */
int __cdecl Sim_100b487e(void);
/* Dword +0x28 of entry `index` of that table; 0 when index is negative or not below dword 0x1017d218. */
uint32_t __cdecl Sim_100b53bb(int index);
/* Index of the first of the signed obj[0] dwords of the array at obj+8 that equals value; -1 if none. */
int __cdecl Sim_100bda0b(const void *obj, uint32_t value);

#endif
