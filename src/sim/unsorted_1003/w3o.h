/*
 * Round 3 (letter o): bounding test of a vertex index list (0x1003196f).
 */
#ifndef HG_SIM_UNSORTED_1003_W3O_H
#define HG_SIM_UNSORTED_1003_W3O_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif


/*
 * `list`: word at +2 = number of vertices, dword at +4 = offset (from list) of that many index bytes;
 * `scene`: vertices of 0x58 bytes from scene+0x18 with the floats x, y, z at +0xc, +0x10, +0x14.
 * Takes the minimum and maximum of x, y, z over the listed vertices (start values 3.4e38 / -3.4e38).
 * Returns 0 if min x > xHi, max x < xLo, min z > zHi or max z < zLo (or a NaN made a "less" test true);
 * otherwise stores min y and max y (raw dwords) through yMin, yMax and returns 1.
 */
int __cdecl Sim_1003196f(void *list, void *scene, float xLo, float xHi, float zLo, float zHi, uint32_t *yMin, uint32_t *yMax);

#endif
