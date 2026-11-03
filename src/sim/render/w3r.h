/*
 * Round 3 (letter r), renderer-facing: screen-space tests and projection with the
 * viewport values stored at 0x10185bd0..0x10185bf4, a plugin Render call that takes
 * raw pixels, and the call of the registered polygon callback. The x87 arithmetic
 * of the original is done in double here, like in w2n.h.
 */
#ifndef HG_SIM_RENDER_W3R_H
#define HG_SIM_RENDER_W3R_H

#include <stdint.h>

#include "../simmem.h"

/*
 * True (1) when sum > -c, where sum = fx * [p+0x58] / [0x10185be4] + fy * [p+0x64] / [0x10185be8],
 * fx = x - [0x10185bd4], fy = [0x10185bf4] - y (screen x, y relative to the centre, y upwards)
 * and c = [p+0x70]. A NaN gives 0.
 */
int __cdecl Sim_10052e60(int x, int y, const uint8_t *p);
/* The inverse in y: screen x for screen row y on the plane p, trunc(-(fy * [p+0x64] / [0x10185be8] + [p+0x70]) * [0x10185be4] / [p+0x58]) + [0x10185bd4]. */
int __cdecl Sim_10052f3a(int y, const uint8_t *p);
/*
 * Projects the world point v[0..2] to the screen in place: (x, y) become the screen
 * position as whole-pixel floats, z the depth in camera space. Camera position
 * [0x10185c48],[0x10185c44],[0x10185c4c], rotation rows [0x10185bfc..0x10185c1c], divisor
 * [0x1036f1d8]. Returns 1 when the point is not behind the camera (z sign bit clear) and the
 * pixel lies inside the rectangle [0x10185bd0],[0x10185bec] x [0x10185bf0],[0x10185be0]
 * (limits included), else 0.
 */
int __cdecl Sim_10090099(float *v);

/*
 * Passes a polygon with raw 8-bit pixels (width, height clamped to 0x100 and to
 * width) to the plugin's Render (slot 0x109e3d30; op 6,
 * second argument `surface`). First loads the luma table of the cached file at
 * [0x1015cff0] into 0x10185c60 (rows reversed, like Render_LoadLumaTable but without
 * loading) and passes it to SetLumaTable the first time ([0x1017e610]). The colour-key
 * dword of the texture description is never set (stack garbage in the original too);
 * the last two arguments are not used. Returns nothing.
 */
void __cdecl Render_SubmitVerts_100d1e10(void *surface, void *pixels, short width, short height, int count, void *verts, int unused1, int unused2);

/* Calls the function registered at [0x1017d15c] with one argument and returns its result. */
int __cdecl Sim_100a78d9(int arg);

#endif
