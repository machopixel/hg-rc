/*
 * Round 14: game functions of HGSIM.DLL around 0x100d0000..0x100dffff that were left original for a stack overflow,
 * settled by the shipped data (DECISIONS.md, "Round 14").
 */
#ifndef HG_SIM_UNSORTED_100D_W14_H
#define HG_SIM_UNSORTED_100D_W14_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * Places the object of an OBJ chunk of a world file (chunk: words +8 key, +0xa parent, +0xc state, ints +0xe..+0x2e
 * scale, angles (times [0x1012bd88]) and position, +0x34 colour, word +0x38 for the reports, the name at +0x3a).
 * Its WTB resource (0x100ba7d5, [0x1014cedc]) must load (else report 0x38). With add (and [0x101507f8] != -1) it
 * only becomes a scene entry (Sim_100b39c3, parent from Sim_100b38c5). With deferred and room in the list
 * ([0x101c42a0] < [0x101a1160]) it becomes a unit (Sim_1003aa7d, keys in the lists [0x1017d1f8] / [0x1017d1f4]).
 * Otherwise the model is read (Sim_100e2a31; a name starting with 'o' also loads its SDF part list: "oc..." each
 * part as its own model, other "o..." added to the main one with Sim_100e3ba4), coloured, given its state (0..8,
 * else 4; names starting "tl1m" / "TL1M" 8) and key (Sim_100b367e), and placed: under the world for parent -2
 * (matrix from Sim_10060656), else as a node (and the parts as nodes) under the parent's node (Sim_100b36d8 /
 * Sim_100096ee).
 */
void __cdecl Sim_100db78b(uint8_t *chunk, uint32_t unused, uint32_t owner, int32_t add, int32_t deferred,
                          uint32_t side);

#endif
