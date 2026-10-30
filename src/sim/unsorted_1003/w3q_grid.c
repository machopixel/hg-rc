#include "w3q.h"
#include "../unsorted_1002/w3q.h"
#include "../render/w2n.h"
#include "../util/w3q.h"

#define GRID_K1 GF32(0x101112e0)
#define GRID_K2 GF32(0x101112e4)
#define POLY_MIN GF32(0x101112e8)

/* Cell number of a coordinate: (value - origin + K1) / K2 truncated. */
static int CellIndex(float value, const void *grid, unsigned originOffset)
{
    double d = ((double)value - (double)F32A(grid, originOffset) + (double)GRID_K1) / (double)GRID_K2;

    return Render_Ftol(d);
}

/* The cell at column ix, row iz. */
#define CELL(grid, ix, iz) (BYTES(grid) + (uint32_t)(ix) * 0xe0 + 0x24 + (uint32_t)(iz) * 0x20)

/* The point is inside the x and z ranges at +0/+4 and +0x10/+0x14 of `box` (a grid or a cell). */
static int InRanges(const void *box, float x, float z)
{
    if (X87_BELOW(x, F32A(box, 0)))
        return 0;
    if (X87_ABOVE(x, F32A(box, 4)))
        return 0;
    if (X87_BELOW(z, F32A(box, 0x10)))
        return 0;
    if (X87_ABOVE(z, F32A(box, 0x14)))
        return 0;
    return 1;
}

// FUNCTION: HGSIM 0x100310e2
int __cdecl Sim_100310e2(const void *grid, const void *mesh, float x, float y, float z)
{
    int ix, iz;
    const uint8_t *cell;
    const uint8_t *verts;
    uint32_t node;

    if (grid == 0 || mesh == 0)
        return 0;
    if (!InRanges(grid, x, z))
        return 0;
    if (X87_ABOVE(y, F32A(grid, 0xc)))
        return 2;
    if (X87_BELOW(y, F32A(grid, 8)))
        return 3;
    ix = CellIndex(x, grid, 0x18);
    iz = CellIndex(z, grid, 0x20);
    if (ix < 0 || ix >= 7 || iz < 0 || iz >= 7)
        return 0;
    cell = CELL(grid, ix, iz);
    if (U32(cell, 0x1c) == 0 || U32(cell, 0x18) == 0)
        return 0;
    verts = BYTES(mesh) + 0x18;
    for (node = U32(cell, 0x1c); node != 0; node = U32(node, 0)) {
        const uint8_t *poly = BYTES(U32(node, 4));
        float height;

        if (poly == 0)
            continue;
        if (!X87_ABOVE(F32A(poly, 0x1c), POLY_MIN))
            continue;
        if (!Sim_1002e9f5(poly, verts, x, z))
            continue;
        return Sim_1002d8be(poly, verts, x, y, z, &height) ? 3 : 2;
    }
    return 2;
}

// FUNCTION: HGSIM 0x100309aa
int __cdecl Sim_100309aa(const void *grid, const void *mesh, float x, float y, float z, float *out)
{
    int ix, iz;
    const uint8_t *cell;
    const uint8_t *verts;
    uint32_t node;

    if (grid == 0 || mesh == 0 || out == 0)
        return 0;
    if (!InRanges(grid, x, z))
        return 0;
    ix = CellIndex(x, grid, 0x18);
    iz = CellIndex(z, grid, 0x20);
    if (ix < 0 || ix > 7 || iz < 0 || iz > 7)
        return 0;
    if (ix == 7)
        ix = 6;
    if (iz == 7)
        iz = 6;
    cell = CELL(grid, ix, iz);
    if (!InRanges(cell, x, z)) {
        int found = 0;
        int a, b;

        for (a = ix - 1; a < ix + 2 && !found; a++) {
            if (a < 0 || a >= 7)
                continue;
            for (b = iz - 1; b < iz + 2 && !found; b++) {
                if (b < 0 || b >= 7)
                    continue;
                cell = CELL(grid, a, b);
                if (InRanges(cell, x, z))
                    found = 1;
            }
        }
        if (!found)
            return 0;
    }
    if (U32(cell, 0x1c) == 0 || U32(cell, 0x18) == 0)
        return 0;
    verts = BYTES(mesh) + 0x18;
    for (node = U32(cell, 0x1c); node != 0; node = U32(node, 0)) {
        const uint8_t *poly = BYTES(U32(node, 4));

        if (poly == 0)
            continue;
        if (X87_ABOVE(F32A(poly, 0x1c), POLY_MIN) && Sim_1002e9f5(poly, verts, x, z)) {
            Sim_1002d8be(poly, verts, x, y, z, out);
            return 1;
        }
    }
    *out = 0;
    return 1;
}

// FUNCTION: HGSIM 0x100349b1
void __cdecl Sim_100349b1(uint32_t value, uint32_t row)
{
    uint8_t *p;
    unsigned digit;

    if (row > 0x19)
        return;
    p = (uint8_t *)(uintptr_t)((row + 1) * 0xa0 + 0xafffe);
    for (digit = 0; digit < 4; digit++) {
        *p = *SIM_PTR(uint8_t, 0x10139fa8 + (value & 0xf));
        p -= 2;
        value >>= 4;
    }
}
