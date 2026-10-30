/*
 * Wave 2 (letter i) additions to unsorted_1004: functions around 0x10040000..0x1004e000: record copies and the builders of the packed words that Sim_1004bf79 .. Sim_1004cde2 take apart.
 * Plain C, original cdecl convention. Names carry the original address
 * because the code alone does not say what the data is for.
 */
#ifndef HG_SIM_UNSORTED_1004_W2I_H
#define HG_SIM_UNSORTED_1004_W2I_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

#include "../unsorted_100d/w2i.h"

/* ---- w2i_misc.c ---- */
/*
 * p points to a small record: byte +2 is an index into the table 0x109e8b40,
 * word +3 a bit mask. Does nothing unless that table entry exists, is not the
 * entry [0x1017b8c8] (also not when its dword +0x14 is 3 and +0xc is
 * [0x1017b8c8]) and the index is below the signed dword [0x1015dc64]. For
 * each of the 8 low bits of the mask that is set, the matching 0x128-byte
 * record of the entry's block (block = dword +0x28; its list pointer is at
 * +0x84) gets state 2 (dword +8), the block's selected record (+0x4fc)
 * becomes that one and the flag dword 0x101853c0[bit] is set; the mask word is
 * shifted down as it goes. Ends with Sim_100d63fa(block).
 */
void __cdecl Sim_1004044c(void *p);
/* Copies fields of the record src (+0x180.., +0x198, +0x1a0..+0x1ae, +0x23e, +0x242) into the packed record dst (+0 .. +0x1e). */
void __cdecl Sim_10040645(void *dst, void *src);
/* Copies byte +4 and byte +2 of p to bytes +0x138 and +0x137 of the record at dword 0x1015d004. */
void __cdecl Sim_10040f2d(void *p);
/* Sets the 25 dwords from 0x109b8b60 to -1. */
void __cdecl Sim_1004f0a6(void);

/* ---- w2i_pack.c ---- */
/* The inverse of the nibble getters of fns.h: (a - 6) | (b - 9) << 4 | (c - 0xb) << 8 | (d != 0) << 12. */
int __cdecl Sim_1004bf28(int a, int b, int c, int d);
int __cdecl Sim_1004c501(int a, int b, int c, int d, int e); /* (a-0xe) | (b-0xe)<<4 | (c-0xe)<<8 | (d-0x16)<<12 | (e-0x13)<<16 */
int __cdecl Sim_1004c8a5(int a, int b, int c, int d, int e); /* (a-0x18) | (b-0x18)<<4 | (c-0x18)<<8 | (d-0x1d)<<12 | (e-0x20)<<16 */
/* Bit i of the result is set when argument i (i = 0..7) is non-zero; arguments 8, 9, 10 set bits 9, 10, 11. */
int __cdecl Sim_1004ccd9(int a0, int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8, int a9, int a10);
int __cdecl Sim_1004c2f7(int hi, int lo); /* hi << 16 | (lo & 0xffff) */
int __cdecl Sim_1004d257(int lo, int hi); /* hi << 16 | (lo & 0xffff) */
/*
 * Kind of record `index` (0xAA bytes each) of the table whose dword at
 * 0x1011168c + index * 0xaa is: 0 -> 0, 1 -> 1, 3 -> 2, 4 -> 3, 2 -> 4,
 * 5 .. 10 -> 5, otherwise -1 (signed); then returns the packed word that the
 * builder of that kind makes (kind 0: Sim_1004bf28(6, 0xa, 0xb, 0); 1:
 * Sim_1004c2f7(0x2d, dword at 0x101116be + index * 0xaa); 2: Sim_1004c501(14,
 * 14, 14, 0x16, 0x13); 3: Sim_1004c8a5(0x18, 0x18, 0x18, 0x1d, 0x20); 4:
 * Sim_1004ccd9(0, 1, 0 ...); 5: Sim_1004d257(0x2f, 0)); 0 for kind -1.
 */
int __cdecl Sim_1004bac0(int index);
/* Reads the dword at 0x101116c6 + index * 0xaa: 0xf..0x12 -> 4, 0x13..0x15 -> 3, 0x16, 0x17 -> 6, 0x18 -> 1, 0x19 -> 2, 0x1a -> 3, else 1. */
int __cdecl Sim_1004d285(int index);

#endif
