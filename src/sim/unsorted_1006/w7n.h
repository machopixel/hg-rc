/*
 * Round 7, letter n: functions at 0x10063415..0x10068e77 (a timed box on the screen, a sphere-tree search and a
 * box-against-box corner test). The x87 arithmetic is done in double where the original keeps a double (control
 * word 0x27f, 53-bit precision) and rounded to float exactly where the original stores a float, so the results are
 * bit-identical; see util/w3o.h for the comparison idioms.
 */
#ifndef HG_SIM_UNSORTED_1006_W7N_H
#define HG_SIM_UNSORTED_1006_W7N_H

#include <stdint.h>
#include "../simmem.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * The box at 0x10144f28 (centre +0/+4, half sizes +8/+0xc, +0x1c colour, see Sim_1006326b). A new id (not
 * [0x10144f4c]) restarts it at (x, y) with both half sizes [0x109e3e1c] >> 1, [0x10144f48] = 1 and colour
 * [0x10144f44] = 0xc / 0x14 / 4 for kind 0 / 2 / other, draws it unless `noDraw`, and returns 0. The same id with
 * noDraw 0: while the half height is positive draws it, then shrinks both half sizes by
 * trunc([0x1015dc80] * float [0x10144f38]) and moves it to (x, y); returns 1 unless [0x10144f84] > 0. With noDraw
 * set it returns 0.
 */
int __cdecl Sim_10063415(int noDraw, uint32_t id, uint32_t x, uint32_t y, int kind);
/*
 * Searches the tree of `obj` (sphere centre +0x60.., radius +0x6c; the children's bounding sphere +0x44..+0x50,
 * the child list [obj+0x28]+4, linked through +8, each child's object at +0x8c) for the object nearest to the
 * point (x, y, z) whose sphere, grown by r, contains it. *dist, *limit and *dx, *dy, *dz receive the distance, the
 * grown radius and the offset from the centre of the best one. Returns that object or 0; objects with flag 0x800
 * (word +0) are not taken as leaves.
 */
void *__cdecl Sim_10067356(uint8_t *obj, float r, float *dist, float *limit, float x, float y, float z, float *dx,
                           float *dy, float *dz);
/*
 * Tests the eight corners of b's box (+0x284) moved into a's frame (b's angles and position from its object
 * [*b+0x48], then minus a's position and the inverse of a's angles) against a's box. On the first corner inside it
 * stores the corner in *outX, *outY, *outZ (when all three are given) and returns 1; else 0.
 */
int __cdecl Sim_10068e77(uint8_t **a, uint8_t **b, float *outX, float *outY, float *outZ);

#endif
