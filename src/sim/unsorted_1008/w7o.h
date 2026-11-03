/*
 * Round 7, letter o, functions at 0x1008xxxx: the grid cell of a scene object and the frame of a bitmap.
 * Floating point is computed in double and truncated with Render_Ftol (bit-exact, see util/w3o.h).
 * Callees that are not converted (or patched by another hook file) are called through their original address.
 */
#ifndef HG_SIM_UNSORTED_1008_W7O_H
#define HG_SIM_UNSORTED_1008_W7O_H

#include <stdint.h>
#include "../simmem.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Patched by another hook file (0x100ef727): the seven-argument line helper (bitmap, x0, y0, x1, y1, 0, colour). */
#define W7O_CALL_100ef727 \
    SIM_FN(void(__cdecl *)(void *, int32_t, int32_t, int32_t, int32_t, int32_t, uint32_t), 0x100ef727)
#define W7O_CALL_100e86a1 \
    SIM_FN(void(__cdecl *)(void *, int32_t, int32_t, int32_t, int32_t, int32_t, uint32_t), 0x100e86a1)

/*
 * Grid cell of a scene object (word +0 flags, +0x28 its node, +0x8c/+0x90 its cell): when the grid is on
 * ([0x1017c8c4]) and the object is not out of the grid (flag 0x1000), takes the node position (Sim_100a9cfb),
 * column = trunc(trunc((x + [0x1014b8e0]) / [0x1012b8e0]) + [0x1012b8e4]), row the same with z and [0x1014b8e4];
 * when the cell changed: Sim_10088b24(node, column, row) inside the 9 x 9 grid, else Sim_10088986(node).
 */
void __cdecl Sim_10088a2d(uint16_t *obj);

/*
 * With w = bmp[3] - bmp[1] and h = bmp[4] - bmp[2] (dwords): the four sides (0,0)-(w,0), (w,0)-(w,h),
 * (w,h)-(0,h), (0,h)-(0,0) through 0x100ef727 with the 16-bit colour word [0x101659e0 + 2 * colour].
 */
void __cdecl Sim_1008d4f1(int32_t *bmp, int32_t colour);

#endif
