/*
 * Wave 2 (batch f) functions of HGSIM.DLL in 0x1005xxxx: constant returns and
 * setters of single globals. Plain C, original cdecl convention.
 */
#ifndef HG_SIM_UNSORTED_1005_W2F_H
#define HG_SIM_UNSORTED_1005_W2F_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Returns 0. */
int __cdecl Sim_10057a51(void);
/* Stores v clamped to 0..0x80 (anything below 1 becomes 0) in the dword 0x10143fbc. */
void __cdecl Sim_10058549(int v);
/* With -1 returns the dword 0x10143fd8; otherwise stores v there and returns it. */
int __cdecl Sim_100585c4(int v);
/* Stores the four arguments in the dwords 0x109b16c4, 0x109b16c8, 0x109b16c0 and 0x109afdcc. */
void __cdecl Sim_10058716(uint32_t a, uint32_t b, uint32_t c, uint32_t d);
/* Stores a in the dword 0x101668b4. */
void __cdecl Sim_1005873d(uint32_t a);

#endif
