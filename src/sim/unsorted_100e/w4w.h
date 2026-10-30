/*
 * Round 4, letter w (area 0x100e4xxx).
 */
#ifndef HG_SIM_UNSORTED_100E_W4W_H
#define HG_SIM_UNSORTED_100E_W4W_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * Clears one dword of the two 256 byte tables: with bit 7 of the low word of `index` clear the dword at
 * 0x10153280 + 4*index, otherwise the dword at 0x10153180 + 4*(index with the low word's bit 7 flipped).
 */
void __cdecl Sim_100e469b(uint32_t index);

#endif
