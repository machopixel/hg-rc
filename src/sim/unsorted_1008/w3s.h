/*
 * Wave 3S, original 0x10088775..0x1008c5e0: the 9x9 grid of cells at 0x1017ba28 (row stride 0x18c,
 * cell 0x2c; the cell's dword +0x24 is what Sim_10088775 returns) and a palette rotation. Plain C,
 * original cdecl convention.
 */
#ifndef HG_SIM_UNSORTED_1008_W3S_H
#define HG_SIM_UNSORTED_1008_W3S_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * Grid column = trunc(trunc((x + [0x1014b8e0]) / [0x1012b8e0]) + [0x1012b8e4]) and row from z the
 * same way with [0x1014b8e4]. For a column and row in 0..8 returns the dword +0x24 of the cell
 * (0x1017ba4c + column * 0x18c + row * 0x2c), else the dword [0x1017c8c8].
 */
int __cdecl Sim_10088775(float x, float z);
/*
 * When the word obj+0 has bit 0x1000 and the high nibble of the word at obj+2 is not 7, and obj+4 is
 * non-zero: clears the bit, Sim_1008a871(obj), and relinks obj after the grid cell its
 * position (Sim_100a9cfb on obj+0x28: x = +0x70, z = +0x78) falls in with Sim_1008a8f5 (columns and rows in
 * obj+0x8c / +0x90); outside the grid they become 9999 and the anchor is 0x1017b9b0; with [0x1017c8c4] zero the
 * anchor is 0x1017b988. Does nothing for NULL.
 */
void __cdecl Sim_1008a46e(void *obj);
/*
 * obj+4 points to RGB triplets, obj+8 (byte) is the first triplet, and the dword at obj+9 (unaligned)
 * the number of triplets to rotate: moves each to the place before it and the first one to the end,
 * then calls method +0x18 of the display driver table that [0x1015e688] points to with obj+4.
 */
void __cdecl Sim_1008c5e0(void *obj);

#endif
