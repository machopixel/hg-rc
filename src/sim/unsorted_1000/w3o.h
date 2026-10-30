/*
 * Round 3 (letter o) functions around 0x10008ed0..0x10009eb3: a field setter
 * and the sphere (centre + radius) of an object with the three tests that use
 * it. The sphere is made of the floats at obj+0x44, +0x48, +0x4c (centre) and
 * +0x50 (radius) in Sim_1000946e, Sim_10009ca8 and Sim_10009eb3, and of
 * +0x60, +0x64, +0x68 and +0x6c in Sim_10009d7e. A ray is the segment of
 * unsorted_1009/w3o.h (start, delta, unit direction, length at +0x30).
 * Functions that leave a float in st(0) are declared with the C type of what
 * they leave there.
 */
#ifndef HG_SIM_UNSORTED_1000_W3O_H
#define HG_SIM_UNSORTED_1000_W3O_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* If the pointer at obj+0x30 is not NULL: the 16-bit word at its +0x14 = value truncated to int. */
void __cdecl Sim_10008ed0(void *obj, float value);

/*
 * Returns obj+0x50; stores obj+0x44, +0x48, +0x4c (raw dwords) through the
 * three pointers that are not NULL. For a NULL obj returns the float at
 * 0x10111018 and stores nothing.
 */
float __cdecl Sim_1000946e(void *obj, uint32_t *x, uint32_t *y, uint32_t *z);

/*
 * With d = centre - (x, y, z) and r the radius: when |d.x|, |d.y|, |d.z| are
 * all <= r and r * r - (d.z * d.z + d.x * d.x + d.y * d.y) is greater than the
 * float at 0x10111018 returns Sim_10084a94(d), else the float at 0x1011101c.
 */
double __cdecl Sim_10009ca8(void *obj, float x, float y, float z);

/*
 * Ray against the sphere at obj+0x60, radius r. With d = centre - ray start:
 * c = d.d - r * r, b = d . delta (ray +0x18..) / length (ray +0x30), q = b * b - c,
 * t = b - r. Returns, testing in this order and by the sign bit of the value:
 *   the float at 0x1011101c  if r < the float at 0x10111020
 *   the float at 0x10111024  if c is negative
 *   the float at 0x1011101c  if the dot product before the division is negative
 *   the float at 0x1011101c  if q is negative
 *   the float at 0x10111018  if t is negative
 *   t if the ray's length (Sim_1009cbde) is greater than t, else 3.4e38 (0x7f7fc99e).
 */
float __cdecl Sim_10009d7e(void *obj, void *ray);

/* The same test against the sphere at obj+0x44 (radius at +0x50). */
float __cdecl Sim_10009eb3(void *obj, void *ray);

#endif
