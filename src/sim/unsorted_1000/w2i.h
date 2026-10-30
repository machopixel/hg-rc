/*
 * Wave 2 (letter i) additions to unsorted_1000: small accessors and an empty function around 0x10000000..0x1000a000.
 * Plain C, original cdecl convention. Names carry the original address
 * because the code alone does not say what the data is for.
 */
#ifndef HG_SIM_UNSORTED_1000_W2I_H
#define HG_SIM_UNSORTED_1000_W2I_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

void __cdecl Sim_100071f6(void); /* does nothing */
/* The 16-bit word at +0x24 of obj. */
void __cdecl Sim_1000942e(void *obj, int16_t value);
/*
 * rec = U32(obj, 0x30) + index * 0x58 + 0x18; does nothing when that pointer
 * is 0. Each of a, b, c that is not NULL receives the dword at rec + 0xc,
 * +0x10 and +0x14.
 */
void __cdecl Sim_10009558(void *obj, int index, uint32_t *a, uint32_t *b, uint32_t *c);

#endif
