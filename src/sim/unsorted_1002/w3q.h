/*
 * Round 3 (letter q) functions of HGSIM.DLL around 0x1002b000..0x1002f000:
 * point tests against a mesh of polygons, and two functions that set up
 * object widgets and call their methods. Names carry the original address
 * because the code alone does not say what the fields mean.
 *
 * Mesh layout, as far as these functions use it (mesh = the dword at +0x30 of
 * an object):
 *   +6 (short)   number of polygon records
 *   +8 (dword)   offset from the mesh to the polygon records, 0x28 bytes each
 *   +0x18        vertex array, 0x58 bytes per vertex, x/y/z floats at +0xc/+0x10/+0x14
 * Polygon record: word +2 = number of vertices, dword +4 = offset from the
 * record to a byte list of vertex numbers, floats +0x18/+0x1c/+0x20 = plane
 * normal (used by Sim_1002d8be; +0x1c is also compared with the float at 0x101112c0).
 * Floating point is done as the original's x87 did it (see util/w3q.h).
 */
#ifndef HG_SIM_UNSORTED_1002_W3Q_H
#define HG_SIM_UNSORTED_1002_W3Q_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * Plane of a polygon at (x, z): out = y of the plane through the polygon's first
 * vertex, (vy - ((n.x * (x - vx) + n.z * (z - vz)) + K) / n.y) with K the float at
 * 0x101112c4. Copies the three plane floats to 0x1015dd40, 0x1015dd44, 0x1015dd48.
 * Returns 1 when y is below the result (or unordered with it), else 0.
 */
int __cdecl Sim_1002d8be(const void *poly, const void *verts, float x, float y, float z, float *out);
/*
 * Point (a, b) inside the polygon, in the plane of the vertex x (+0xc) and z (+0x14)
 * for Sim_1002e9f5, of x and y (+0x10) for Sim_1002ed0b: counts how the polygon's edges
 * cross the horizontal line through b to the left and right of a. Returns 1 inside or on
 * an edge, else 0. Polygons with fewer than 3 vertices, or with every vertex on one side
 * of a or b, give 0.
 */
int __cdecl Sim_1002e9f5(const void *poly, const void *verts, float a, float b);
int __cdecl Sim_1002ed0b(const void *poly, const void *verts, float a, float b);
/*
 * Looks through the polygons of obj's mesh (obj+0x30) for the first one with +0x1c above
 * the float at 0x101112c0 that contains (a, c) (Sim_1002e9f5); then
 * returns Sim_1002d8be's answer for it with y = b. 0 when there is none.
 */
int __cdecl Sim_1002d276(const void *obj, float a, float b, float c);
/*
 * The same search over the polygons with +0x1c below the float at 0x101112c0; the answer is 1 when
 * b is above the height of the polygon's plane at (a, c) (height computed with the
 * float at 0x101112c0 as K). Copies the plane floats to 0x1015dd40..48. 0 when none.
 */
int __cdecl Sim_1002d4a7(const void *obj, float a, float b, float c);
/*
 * When the dword obj+0x98 is set: Sim_100310e2(obj[0x98], obj[0x2c], a, b, c) & 1,
 * otherwise Sim_1002d276(obj, a, b, c).
 */
int __cdecl Sim_1002d3f8(const void *obj, float a, float b, float c);
/* Sim_100309aa(obj[0x98], obj[0x2c], a, b, c, d) with a, b, c floats and d a float pointer. */
int __cdecl Sim_1002d479(const void *obj, float a, float b, float c, float *d);

/*
 * Render_Ftol((double)(int)[0x1013b98c] / [0x1011120c] * [0x10111210] + [0x10111214]), the three
 * divisor, factor and addend being floats. The argument is not used; no direct caller is known.
 */
int __cdecl Sim_100219a0(int unused);

/*
 * Widget objects: pointers at 0x109e88e0 + 4 * n, each object holding method pointers (called as
 * method(object, ..)) at +0x10c, +0x110, +0x114, +0x124, +0x12c, +0x130 and rectangle data at
 * +0xd4/+0xdc/+0xe4 (pairs of 16.16 fixed values), the rectangle to scale them by at +0xec.
 *
 * `owner` has the array of eight 0x128-byte records at +0x84. Stores the record number at +0x104 of
 * each, then sets up the widgets 3..10 from the records: +0x114 method with the 0xd6-byte table
 * entry (at 0x10145592) selected by the dword record+4, +0x110 method with the number, callbacks
 * 0x100026b2 / 0x10002c34 at +0x138 / +0x13c, the pairs at +0xd4, +0xdc, +0xe4 (set from constants that
 * depend on the dword 0x101c64f0, 0x199a / 0xcccd for the last) scaled in place with Rect_ScalePoint,
 * and record+4 = -1 for records whose first dword is 0.
 */
void __cdecl Sim_1002b68f(const void *owner);
/*
 * Sim_1002b68f for the widget data of the entry [0x1017b8c8] of the pointer table 0x109e8b40 (its +0x28),
 * then for the 30 widgets 0..29: method +0x10c(obj, dword 0x101381d0[n]), +0x124(obj, dword 0x10138158[n]),
 * +0x12c(obj, 0) and, when the dword 0x109e8960[n] is non-zero, +0x130(obj).
 */
void __cdecl Sim_1002c113(void);

#endif
