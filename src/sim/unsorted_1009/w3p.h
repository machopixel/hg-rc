/*
 * Round 3 function of HGSIM.DLL in 0x1009xxxx (letter p).
 */
#ifndef HG_SIM_UNSORTED_1009_W3P_H
#define HG_SIM_UNSORTED_1009_W3P_H

#include <stdint.h>

#include "../simmem.h"

/*
 * Fills a box record (writes the dwords +0 .. +0x34): floats +0..+0x14 = the six arguments (min x, y, z then max x, y, z), floats
 * +0x18..+0x20 = max - min per axis, dwords +0x24, +0x28, +0x2c, +0x30, +0x34 = 0.
 */
void __cdecl Sim_1009cab0(float *box, float minX, float minY, float minZ, float maxX, float maxY, float maxZ);

#endif
