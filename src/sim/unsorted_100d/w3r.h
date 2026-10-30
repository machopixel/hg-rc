/*
 * Round 3 (letter r), original 0x100d867b and 0x100d8936: filling of the nine records (0x80c bytes
 * each) at 0x10180a00 from caller-built descriptions. A flags dword in the description (at +4 of the
 * short form, +4 of the long one) says which parts are given; the other parts get fixed defaults.
 * Ints in the description become floats in the record.
 */
#ifndef HG_SIM_UNSORTED_100D_W3R_H
#define HG_SIM_UNSORTED_100D_W3R_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * Fills record `index` (0..8, otherwise nothing) from the 0xf4-byte description `src`; flags = src[+4]:
 * 1: +8 dword; 2: +0xc..+0x14 ints -> floats; 4: 0x80 bytes at +0x18; 8: +0x98 dword (default 1);
 * 0x10: +0x9c dword (default 1); 0x20: +0xa8 dword (default 0); 0x40: +0xac..+0xb4 ints -> floats;
 * 0x80: +0xb8 int -> float; 0x100: +0xbc dword and +0xc0..+0xd4 ints -> floats; 0x200: +0xd8..+0xec ints ->
 * floats; 0x400: +0xf0 dword (default 0). Parts that are not selected keep their default (+8 and +0xf0 are
 * cleared, +0x98 and +0x9c set to 1, +0xa8 cleared); the others are left as they are.
 */
void __cdecl Sim_100d867b(const uint8_t *src, int index);
/*
 * Sets all nine records (type dword +0 = 1, or 2 when src[0] == 2; [0x1015245c] gets the same value,
 * [0x1017e684] = 1) from the block `src` (flags = src[+4]) and then applies Sim_100d867b to the
 * 0xf4-byte descriptions at src+0xb4 + 0xf4 * k for the flags 0x10000 << k (k = 0..8, record k).
 * 1: 0x80 bytes at record+0x18 from src+8; 2: +0xbc from src+0x88 and +0xc0..+0xd4 from the ints at
 * src+0x8c..+0xa0 (else all zero); 4: +0xb8 from the int src+0xb0, else 14.0f; +0xf8 = +0xb8 / [0x1012bd64];
 * 8: +0xac..+0xb4 from the ints at src+0xa4..+0xac, else (0, -10000.0f, 0), or -5000.0f in the middle for type 2.
 */
void __cdecl Sim_100d8936(const uint8_t *src);

#endif
