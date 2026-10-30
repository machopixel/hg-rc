/*
 * Round 9, letter h: the group position update (0x100527fa).
 */
#ifndef HG_SIM_UNSORTED_1005_W9H_H
#define HG_SIM_UNSORTED_1005_W9H_H

#include <stdint.h>
#include "../simmem.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* ---- w9h_misc.c ---- */
/*
 * For group index < 16 whose leader idx (dword 0x109b7d40 + index * 0x6c) is below [0x1015dc64]: the leader's record
 * PTRTAB(idx) gets +0x58 = a, +0x5c = b, +0x60 = c, +0x68 = d, its node +0x48 Sim_100a9d47(node, a, b, c),
 * Sim_100aa0da(node, 0, d, 0, 0) and Sim_100aa799(node); every record PTRTAB(i) of the group (dword +8 == index) gets
 * the same with the point Sim_100529a2(i, &x, &y, &z) (x, b, y, z) and returns 1. Always copies the 0x30-byte matrix
 * Sim_100aa863(PTRTAB(idx) +0x48) to 0x109b7d7c + index * 0x6c. For index >= 16 the original reads idx from an
 * uninitialised local (not reproducible; the C uses 0 there), see the report of round 9.
 */
int __cdecl Sim_100527fa(int index, uint32_t a, uint32_t b, uint32_t c, uint32_t d);

#endif
