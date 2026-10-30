/*
 * Wave 3S, renderer-facing: a projection variant and the camera setup. Plain C, original cdecl
 * convention. See w3s_fp.h for how the x87 arithmetic of the original is reproduced.
 */
#ifndef HG_SIM_RENDER_W3S_H
#define HG_SIM_RENDER_W3S_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * Like Render_ProjectVertex (w2n.h) but dividing by the float at 0x1036f1d8 instead of the vertex depth:
 * screen x = trunc(x / d) + [0x10185bd4], y = [0x10185be0] - [0x10185bf0] - (trunc(y / d) + [0x10185bf4]).
 * Quirk kept: a vertex whose byte +0x29 is non-zero (already projected, ANY value) contributes clip bits 0
 * to the accumulated masks 0x101c7680 / 0x101cc2a0 (the latter is therefore cleared); it does not reuse +0x28.
 * Appends the vertex to the list at 0x101c76a0 like Render_ProjectVertex. Returns its argument.
 */
void *__cdecl Sim_1008ff68(unsigned char *vertex);

/*
 * Makes `camera` the current one: copies its fields into the renderer globals 0x10185b60..0x10185c50
 * (projection scales, screen window, near and far planes, orientation vectors and their products with
 * the scales), sets [0x1014cae0] = camera, calls Sim_1005873d(word +0x2a) and Sim_10058716(+0x1c, +0x20,
 * +0x24, word +0x28), and derives the fog factor [0x10143fcc] from [0x10143fc4] and camera+4.
 * hooks_view.c patches the original of this function: it is verified here but must not be hooked.
 */
void __cdecl Sim_100d1f90(void *camera);

#endif
