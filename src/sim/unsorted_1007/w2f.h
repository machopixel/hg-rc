/*
 * Wave 2 (batch f) functions of HGSIM.DLL in 0x1007xxxx. Plain C, original
 * cdecl convention.
 */
#ifndef HG_SIM_UNSORTED_1007_W2F_H
#define HG_SIM_UNSORTED_1007_W2F_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * Only when the dword 0x1015ec58 is non-zero and the byte at +0xa of the structure
 * pointed to by 0x1017b8c4 is 0. Looks at the 0x13f-byte records (count in the
 * dword at 0x10473800 + 0x4fc3a * [0x1013bb40], records from +0x3a): when the
 * bytes at +0x74 sum to exactly 1 and some record with a non-zero +0x74 has
 * byte +1 equal to 2, it scores each row r of the 8x8 word matrix at 0x1016552c
 * (sum of the row, the diagonal element subtracted) and stores in the dword
 * 0x101655ac the row with the best score: -2 on a tie with the best so far at
 * the time, 0 when no row beats -32767.
 */
void __cdecl Sim_1007b02d(void);
/* Returns 1. */
int __cdecl Sim_1007cbb9(void);
/* *[0x1015e698] = [0x1015e9a0]; returns 0. */
int __cdecl Sim_1007f0b4(void);

#endif
