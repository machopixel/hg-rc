/*
 * Round 3 (letter o) functions at 0x10044296 and 0x1004c965.
 */
#ifndef HG_SIM_UNSORTED_1004_W3O_H
#define HG_SIM_UNSORTED_1004_W3O_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif


/*
 * Appends an entry to the ring of ten 0x1c-byte entries at 0x1015ed70 (count in the signed dword
 * 0x1013aec4) while the count is below 10: +0 a, +4 b, +8 c, +0x10 d, +0xc e, +0x14 f and
 * +0x18 = (int)(g * the double at 0x10111558), the count then goes up by one.
 */
void __cdecl Sim_10044296(uint32_t a, uint32_t b, uint32_t c, uint32_t d, uint32_t e, uint32_t f, double g);

/*
 * (int)( float(S) * m0 * m1 * m2 * m3 * m4 ) where S is the dword at 0x101116b2 + a * 0xaa
 * plus the dword at 0x10124eb8 + 0x98 * k for the five k = Sim_1004c909(b), Sim_1004c917(b),
 * Sim_1004c928(b), Sim_1004c939(b), Sim_1004c94a(b), and m is the float at 0x10124ebc + 0x98 * k
 * of the same five; every product is rounded to float.
 */
int __cdecl Sim_1004c965(int a, int b);

#endif
