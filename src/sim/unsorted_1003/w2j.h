/*
 * Wave 2, letter j: leaf functions of HGSIM.DLL around 0x10030000..0x10040000.
 */
#ifndef HG_SIM_UNSORTED_1003_W2J_H
#define HG_SIM_UNSORTED_1003_W2J_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Dword 0x1015e4fc = value. */
void __cdecl Sim_10034877(uint32_t value);
/* Dword 0x10139e50 = value. */
void __cdecl Sim_10034884(uint32_t value);
/* Returns dword 0x109e3d80; the argument is not used. */
uint32_t __cdecl Sim_10035589(uint32_t unused);
/* Dword at 0x109b9e20 + 0x7e * index = value (the table of 0x7e-byte rows read by Sim_100dc286). */
void __cdecl Sim_1003b449(int index, uint32_t value);

#endif
