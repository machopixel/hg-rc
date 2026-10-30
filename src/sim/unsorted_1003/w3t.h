/*
 * Round 3 (letter t) functions of HGSIM.DLL in 0x1003xxxx. Plain C, original
 * cdecl convention. Names carry the original address because the code alone
 * does not say what the data is for.
 *
 * A "vertex list": a word at +4 is the number of vertices (signed), which
 * start at +0x18 and are 0x58 bytes each, with the float x, y, z at +0xc,
 * +0x10, +0x14 of each (the scene vertex of render/w2n.h).
 */
#ifndef HG_SIM_UNSORTED_1003_W3T_H
#define HG_SIM_UNSORTED_1003_W3T_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * Bounds of a vertex list: the smallest and largest x, y and z go to the
 * outputs (raw float bits; the start values are the bits 0x7f7fc99e for the
 * minimum and 0xff7fc99e for the maximum, kept when the list is empty), and the
 * middle of each pair (sum * 0.5) to cx, cy, cz. Does nothing when any
 * argument is null.
 */
void __cdecl Sim_100303ea(const void *list, float *cx, float *cy, float *cz, uint32_t *minX, uint32_t *maxX,
                          uint32_t *minY, uint32_t *maxY, uint32_t *minZ, uint32_t *maxZ);

/*
 * For an object whose vertex list is the pointer at +0x2c and whose block is
 * the pointer at +0x98 (nothing happens when that is null): stores the
 * bounds of the list in the block (minX, maxX, minY, maxY, minZ, maxZ at
 * +0..+0x14, centre x, y, z at +0x18..+0x20), then resets the 7 x 7 cells of
 * 0x20 bytes at +0x24 (row stride 0xe0) to an empty box: three dwords of
 * 0x7f7fc99e (+0, +8, +0x10), three of 0xff7fc99e (+4, +0xc, +0x14), zero at
 * +0x18 and +0x1c.
 */
void __cdecl Sim_10030672(const void *obj);

#endif
