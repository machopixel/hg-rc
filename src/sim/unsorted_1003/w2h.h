/*
 * Wave 2 functions of HGSIM.DLL around 0x10033000..0x1003c000 (letter h).
 * Names carry the original address because the code alone does not say what
 * the fields mean.
 */
#ifndef HG_SIM_UNSORTED_1003_W2H_H
#define HG_SIM_UNSORTED_1003_W2H_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* The dword *p, or -1 for NULL. */
int32_t __cdecl Sim_10033540(const int32_t *p);
/* 0 when a is 0, otherwise 1 for b = 0, 1 or 2 and 0 for any other b. */
int __cdecl Sim_10036820(uint32_t a, int32_t b);
/*
 * Copies count 3-byte entries starting at entry start of the 256-entry table at
 * 0x1015e6a0 to dest. Returns 0, or -1 (nothing copied) when dest is NULL, start
 * is outside 0..255 or count is not in 1..256 - start.
 */
int __cdecl Sim_1003830e(int32_t start, int32_t count, void *dest);
/* Dword at 0x109b9e68 + 0x7e * index = value. */
void __cdecl Sim_1003b45d(int32_t index, uint32_t value);

#endif
