/*
 * Round 3 (letter o) functions around 0x10025dc3..0x1002e6de: the 0x30-byte
 * records at 0x109eeb00 that Sim_10028c6f randomises, and the segment/cell
 * queries built on the object list that Sim_100881dd fills (0x1036f2a0).
 * Segment: see unsorted_1009/w3o.h.
 */
#ifndef HG_SIM_UNSORTED_1002_W3O_H
#define HG_SIM_UNSORTED_1002_W3O_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* The original jumps to its end at the first instruction (the rest is dead code): does nothing. */
void __cdecl Sim_10025dc3(void);

/*
 * Record `index` (0x30 bytes at 0x109eeb00) with a non-zero dword at +4: floats at +8,+0x10,+0xc
 * and +0x14,+0x18,+0x1c get values from six calls of Sim_1009c969 scaled by the floats at
 * 0x10111284, 0x1013abc4, 0x10111288 and 0x1011128c; if the object at +4 has a pointer at +0x8c,
 * +0x24 = its +0x44, +0x28 = its +0x48 + its +0x3c, +0x2c = its +0x4c.
 */
void __cdecl Sim_10028c6f(int index);

/*
 * Record `index` as above: calls Sim_10028c6f, scales the vector at +8 to at most the length of
 * (x, y, z) times the float at 0x10111290 (lengths by Sim_10084a94), then adds (x, y, z) to it.
 * Does nothing without the object at +4 or when (x, y, z) has length exactly the float at 0x1011129c.
 */
void __cdecl Sim_1002977e(int index, float x, float y, float z);

/* Calls the function at +0x40 of the object 0x109e8b40[[0x1017b8c8]] with the pointer at its +0x28, when both exist. */
void __cdecl Sim_1002a0ca(void);

/*
 * Tests the point (x, y, z) against the three kinds of cell objects (type dword at +0x34: 5, 8, 0) that
 * hang (next at +0x18) off each entry of the list Sim_100881dd(x, z) builds, through the original tests at
 * 0x100035df, 0x10001e1f and 0x10002775 (node, x, y, z, &result). Remembers the node of the last hit in
 * [0x109e8230] and the 3 dwords at 0x1015dd40 of the best hit in 0x1015dd4c. Returns the best result
 * (float, in st(0)): see the code for the 3.4e38 limits and the floats at 0x101112c4/c8/cc.
 */
double __cdecl Sim_1002d961(float x, float y, float z);

/* Finds the segment-list entry nearest to (x, y, z) with Sim_1002dcb3 into *found, then runs Sim_1002dd6c on it; 1 if both succeed. */
int __cdecl Sim_1002dc60(float x, float y, float z, void **found);

/* Walks the list at first+0x10 (next at +0x10) and returns the node with the smallest Sim_10009ca8(node, x, y, z) below 3.4e38, or NULL. */
void *__cdecl Sim_1002dcb3(void *first, float x, float y, float z);

/* Calls the function stored at table 0x10138bc0 + 12 * [obj+0x34] with (obj, x, y, z) and returns its result, or 1 if there is none. */
int __cdecl Sim_1002dd6c(void *obj, float x, float y, float z);

/* dir = vec[0..2] - obj+0x44.. (or vec[3..5] - obj+0x44.. unless t is 10.0) into 0x1015dd40.., normalised with Sim_10084c95; first Sim_1009cd36(vec, t) when t > the float at 0x101112c4. */
void __cdecl Sim_1002e6de(void *obj, float *vec, float t);

#endif
