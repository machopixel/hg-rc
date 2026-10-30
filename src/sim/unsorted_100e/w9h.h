/*
 * Round 9, letter h: the end-of-mission tallies (0x100e1398) and the shade table copy of the span filler (0x100e6b29).
 */
#ifndef HG_SIM_UNSORTED_100E_W9H_H
#define HG_SIM_UNSORTED_100E_W9H_H

#include <stdint.h>
#include "../simmem.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* ---- w9h_misc.c ---- */
/*
 * For every record i < [0x1015dc64]: Sim_1005b98c(i) and the word counter [0x1016541a] / [0x1016541e] / [0x1016541c]
 * of Sim_10052be7(i) == 0 / 1 / 2 is incremented. For every entry i < [0x1015dc68] of the 0x50-byte table at
 * 0x109e9ba8: the counter [0x10165420] / [0x10165424] / [0x10165422] of Sim_10052c02(i) == 0 / 1 / 2, and when
 * Sim_100b5386(entry dword +0) gives a record whose dword +0x8c is set, the entry's dword +0xc = trunc(float +0x50 of
 * that block).
 */
void __cdecl Sim_100e1398(void);
/* Copies 64 dwords from src to the table at 0x101534c0 (forward, dword by dword, like the original's rep movsd). */
void __cdecl Sim_100e6b29(const uint32_t *src);

#endif
