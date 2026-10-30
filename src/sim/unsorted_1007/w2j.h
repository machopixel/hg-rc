/*
 * Wave 2, letter j: leaf functions of HGSIM.DLL around 0x10070000..0x10080000.
 */
#ifndef HG_SIM_UNSORTED_1007_W2J_H
#define HG_SIM_UNSORTED_1007_W2J_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * Sets up the buffers of a record: obj+0x84 = obj+0xc51, obj+0x88 = that+0x940,
 * obj+0x8c = that+0x3fc, then initialises the 8 entries of 0x128 bytes that
 * start at obj+0x84's block and the 0x24 entries of 0x24 bytes at obj+0x8c's.
 */
void __cdecl Sim_10072e35(void *obj);
/* The last of the obj[0xc] dwords of the array at obj+8, or 0 when obj[0xc] <= 0. */
uint32_t __cdecl Sim_100765f6(const void *obj);
/* obj+0xc = 0. */
void __cdecl Sim_10076667(void *obj);

#endif
