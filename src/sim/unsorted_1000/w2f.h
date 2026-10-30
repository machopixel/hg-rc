/*
 * Wave 2 (batch f) functions of HGSIM.DLL in 0x1000xxxx: small accessors and
 * setters of the object, record and table structures. Plain C, original cdecl
 * convention.
 */
#ifndef HG_SIM_UNSORTED_1000_W2F_H
#define HG_SIM_UNSORTED_1000_W2F_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * Table of 14-byte entries at 0x10ac1b00 (512 entries). Fills entry a: +0 = b
 * (when b is -1, the dword at 0x1015cfe0 and the flag 0x1015cfe4 is set to 1),
 * +8 = 1, +6 = 1, +2 = 0, and +4 = 0x2d when the dword at 0x1015cfe8 is non-zero.
 * Returns 0 when a or b is outside 0..0x1ff, else 1.
 */
int __cdecl Sim_10006db8(int a, int b);
/* *p = (v & 0x7ef0) | (*p & 0x810f) | 0x8000; does nothing for a null p. */
void __cdecl Sim_100093be(uint16_t *p, uint32_t v);
/*
 * obj+0x30 points to a block of 0x58-byte records starting at +0x18. Stores the
 * first three dwords of record idx through the non-null pointers o1, o2, o3;
 * does nothing when obj+0x30 is 0.
 */
void __cdecl Sim_100095b6(void *obj, int idx, uint32_t *o1, uint32_t *o2, uint32_t *o3);
/* 1 when the dword at +0x10 of the 0xd6-byte table entry 0x10145508 + 0xd6 * (record idx of obj+0x84 +4) equals val; obj is checked for null. */
int __cdecl Sim_1000c6e3(void *obj, int idx, int val);

#endif
