/*
 * Round 3 functions of the renderer-facing math of HGSIM.DLL (letter p): fog factor, rotation of the light
 * vectors, visibility test of an object against the near/far plane and the sides of the view, and a
 * ground square drawn through the display driver slot. Float work is done in double like the original's
 * x87 code (53-bit precision) and rounded to float where the original stores a float.
 * The camera matrix/position globals used: matrix 0x10185c20..0x10185c40 (three rows of three floats),
 * position 0x10185c48 (x), 0x10185c44 (y), 0x10185c4c (z), near 0x10185c50, far 0x10185bf8.
 */
#ifndef HG_SIM_RENDER_W3P_H
#define HG_SIM_RENDER_W3P_H

#include <stdint.h>

#include "../simmem.h"

/*
 * Fog factor of camera depth z (1.0 = no fog .. 0.0 = full fog) from the fog start float 0x10143fc8, far float
 * 0x10185bf8, and with a distance term made of float 0x10143fc4 and `arg` and the floor 0x101668cc. Returned in st(0)
 * by the original (Render_FogLevel calls it); here a float.
 */
float __cdecl Render_FogFactor_10058ac2(float z, float arg);

/*
 * Rotates the vector (dwords 0x109b16c4, 0x109b16c8, 0x109b16c0, minus the translation when dword 0x109afdcc is 0) and the
 * vectors of the linked list at [0x101668d0] (next pointer at +0, vector floats at +0x14 (minus the translation),
 * result floats at +0x20) by the 3x3 matrix m (floats +0..+0x20, row sums as in the code) with translation m+0x24..m+0x2c;
 * the first result goes to 0x10143fc0, 0x101668b8, 0x101668bc and the translation is copied to 0x101668c0..0x101668c8.
 */
void __cdecl Sim_1005874a(const void *m);

/*
 * Visibility of a sphere (centre floats +0x44..+0x4c, radius float +0x50) of an object whose word +0 has bit 12 clear:
 * 0 visible, 1 when bit 12 is set, 4 nearer than the near plane, 5 beyond the far plane, 6 or 7 outside the view on one
 * axis. Leaves the camera depth of the centre in float 0x101c7aa0.
 */
int __cdecl Sim_1008fdd7(const void *obj);

/* 1 when bit 8 of word +2 selects (word +2 & 0xf00) == 0x100 and the entry word +0x24 of the pointer table has bits 0x16 in its word +0x1c, or the nibble (word +2 & 0xf0) is 0x30 or 0x70; else Sim_1008fdd7(obj). */
int __cdecl Sim_1009b47d(const void *obj);

/*
 * Clips the square of half-size `half` centred on (x, z) at height y (four scene vertices of 0x58 bytes, y raw) with Render_ClipPolygon and
 * the camera's near/far, and when 3 or more vertices remain passes them (screen position, depth, 1/1/1 uv-like floats,
 * fog byte from Render_FogLevel(depth, y) in the top byte of +0x14, +0x10 = 0xf0) to the display driver slot 0x1014cb4c as (count, vertices, 0xf0).
 */
void __cdecl Sim_100ad5b7(float x, float y, float z, float half);

#endif
