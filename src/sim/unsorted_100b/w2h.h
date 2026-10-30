/*
 * Wave 2 functions of HGSIM.DLL around 0x100b3000..0x100bf000 (letter h).
 * Names carry the original address because the code alone does not say what
 * the fields mean.
 */
#ifndef HG_SIM_UNSORTED_100b_W2H_H
#define HG_SIM_UNSORTED_100b_W2H_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * The records of 0x88 bytes at 0x101a2100 number 0x402. When [0x1017d21c] is 0 it first
 * resets all of them with Sim_100b4149. Then, if the signed dword [0x1017d218] is
 * below 0x402 and the signed dword [0x101a1168] below the signed dword [0x101c42a4]:
 * stores [0x1017d218] in the dword array at [0x1017d204] and value in the one at
 * [0x1017d208], both at index [0x101a1168], increments [0x1017d218] and [0x101a1168]
 * and returns the old [0x1017d218]. Returns -1 otherwise.
 */
int32_t __cdecl Sim_100b391c(uint32_t value);
/* Dword +0x28 of the record i (0x101a2100 + 0x88 * i). */
uint32_t __cdecl Sim_100b39af(int32_t i);
/*
 * Resets the record i: byte +0 = 0, dwords +0x10, +0x14, +0x20, +0x24 = -1, dwords
 * +0x18, +0x1c, +0x28, +0x2c, +0x3c, +0x40, +0x44, +0x48, +0x4c, +0x50, +0x84 = 0 and
 * dwords +0x30, +0x34, +0x38 = 0x3f800000 (1.0f).
 */
void __cdecl Sim_100b4073(int32_t i);
/* Resets all 0x402 records with Sim_100b4073, then [0x1017d218] = 0 and [0x1017d21c] = 1. */
void __cdecl Sim_100b4149(void);
/* Skips spaces and tabs; returns the first other character (possibly the terminating 0). */
/* Dword obj+4 = 0. */
void __cdecl Sim_100bda90(void *obj);

#endif
