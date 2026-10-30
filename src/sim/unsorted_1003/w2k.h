/*
 * Small leaf functions of HGSIM.DLL around 0x10037306..0x1003d863. Names carry
 * the original address because the code alone does not say what the data means.
 */
#ifndef HG_SIM_UNSORTED_1003_W2K_H
#define HG_SIM_UNSORTED_1003_W2K_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Does nothing. */
void __cdecl Sim_10037306(void);
/*
 * Splits a pixel value into four bytes with per-channel tables: channel i is
 * ((pixel >> shift[i]) << (8 - bits[i])) & 0xff, shift = dwords at 0x109e38a0,
 * bits = dwords at 0x109e3890. The four bytes are returned from the top (channel
 * 0) to the bottom (channel 3).
 */
uint32_t __cdecl Sim_10037680(uint32_t pixel);
/* Dword at 0x1017b8f0 = value. */
void __cdecl Sim_10037c9e(uint32_t value);
/* Saves the dword at 0x1014cb3c to 0x1013abf0, points 0x1014cb3c at the code at 0x10003837 and clears 0x1015ebf4. */
void __cdecl Sim_1003d863(void);

#endif
