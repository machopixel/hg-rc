/*
 * Wave 2 functions of HGSIM.DLL around 0x100fd000..0x100fe000 (letter h).
 * Names carry the original address because the code alone does not say what
 * the fields mean.
 */
#ifndef HG_SIM_UNSORTED_100f_W2H_H
#define HG_SIM_UNSORTED_100f_W2H_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Swaps count bytes between the buffers a and b, one byte at a time; nothing happens when a == b. */
void __cdecl Mem_Swap(uint8_t *a, uint8_t *b, uint32_t count);

#endif
