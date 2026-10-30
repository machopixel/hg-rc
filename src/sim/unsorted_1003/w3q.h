/*
 * Round 3 (letter q) functions of HGSIM.DLL around 0x10030000..0x10035000:
 * point tests against a mesh through a grid of 7 x 7 cells, and a debug
 * routine. Names carry the original address because the code alone does not
 * say what the fields mean.
 *
 * The grid (first argument of the two tests): floats +0/+4 = x range, +8/+0xc =
 * the two limits tested against y, +0x10/+0x14 = z range, +0x18 and +0x20 = x
 * and z origin; cells of 0x20 bytes start at +0x24, seven per column, 0xe0 bytes
 * per column (column = x, row = z, both (coordinate - origin + K1) / K2 with the
 * floats K1 = [0x101112e0], K2 = [0x101112e4], truncated); a cell has its own
 * floats +0/+4 and +0x10/+0x14 (x and z range), a list head at +0x1c (nodes:
 * next at +0, a polygon record of the mesh at +4) and a dword at +0x18 that must
 * be non-zero. Mesh and polygon layout: see unsorted_1002/w3q.h. In both tests
 * the mesh argument is the pointer whose +0x18 is the vertex array.
 */
#ifndef HG_SIM_UNSORTED_1003_W3Q_H
#define HG_SIM_UNSORTED_1003_W3Q_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * Where (x, y, z) is relative to the grid: 0 when the grid or the mesh is null, the point is
 * outside the x or z range, the cell has no data, or no polygon of the cell list is found;
 * 2 when y is above +0xc (or unordered); 3 when y is below +8; otherwise looks through the
 * cell's polygons (with +0x1c above the float at 0x101112e8) for one containing (x, z)
 * (Sim_1002e9f5) and returns 3 when Sim_1002d8be(polygon, mesh + 0x18, x, y, z, ..) is
 * true, else 2. After the whole list without a hit: 2.
 */
int __cdecl Sim_100310e2(const void *grid, const void *mesh, float x, float y, float z);
/*
 * Finds the cell of (x, z) (or a neighbour whose ranges contain the point), then the first polygon
 * of its list that contains (x, z), calls Sim_1002d8be(polygon, mesh + 0x18, x, y, z, out) and
 * returns 1. When the list holds no such polygon, stores 0 in *out and returns 1. Returns 0 when
 * the grid, mesh or out is null, the point is outside the grid, no cell contains it, or the
 * cell has no data.
 */
int __cdecl Sim_100309aa(const void *grid, const void *mesh, float x, float y, float z, float *out);
/*
 * Writes the four lowercase hexadecimal digits of the low 16 bits of `value` (characters from the
 * string "0123456789abcdef" at 0x10139fa8) as single bytes two apart, least significant digit first, ending at
 * address (row + 1) * 0xa0 + 0xafffe and going down, i.e. the last column of line `row`
 * of a text screen at 0xb0000 (monochrome text memory); nothing for row > 0x19.
 * NOTE: that is a raw address of DOS, not valid on Windows; nothing in the build needs it.
 */
void __cdecl Sim_100349b1(uint32_t value, uint32_t row);

#endif
