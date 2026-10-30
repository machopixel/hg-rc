/* Wave 2e, original 0x100787f4..0x1007df6c. */
#ifndef HG_SIM_UNSORTED_1007_W2E_H
#define HG_SIM_UNSORTED_1007_W2E_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * Two identical routines (the second is a duplicate of the first). Copy the 0x68 bytes at
 * 0x1014cae8 to dst, then reset the state: [0x1014cb00] = 0, [0x1014caf0] = 0, [0x1014caf4] = 1,
 * [0x1014caec] = 1, [0x1014cb38] = 0xb00, [0x1014cb34] = 1 and bit 4 of [0x1014caf8] cleared.
 * (0x1014cb34 and 0x1014cb38 are the flag and bit set of unsorted_1003/fns.h.)
 */
void __cdecl Sim_100787f4(void *dst);
void __cdecl Sim_10078857(void *dst);

/*
 * Picks one of the two 0x3c-byte records at 0x10149d28 (dword +0xc is its in-use flag): the
 * first unused one, else among the used ones whose dword +0x10 is not above `limit` the one with
 * the lowest +0x10 (below 0x29a), ties going to the greater +0x34. Without a pick returns 0.
 * Otherwise copies at most 0xff bytes of `text` into the buffer the record points to (+0), ends it
 * with a 0, sets +0xc = 1, +0x10 = limit, +0x34 = Sim_1002ab9d() + ticks and copies `label`
 * (or the string pointed to by [0x109b98c0] when `label` is null or empty) to +0x14; returns 1.
 * When the first record is used, has +0x10 == 0x29a and is not above the limit, the original
 * compares +0x34 against an uninitialised stack value; this version uses 0 there.
 */
int __cdecl Sim_1007df6c(const char *text, const char *label, int ticks, int limit);

#endif
