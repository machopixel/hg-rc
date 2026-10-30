/*
 * Round 18: renderer functions of HGSIM.DLL that read unset stack (DECISIONS.md, "Round 18").
 */
#ifndef HG_SIM_RENDER_W18_H
#define HG_SIM_RENDER_W18_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * Draws the polygon record poly (word +0 kind in the high nibble, word +2 vertex count, dword +4 offset of its byte
 * vertex indexes, normal +0x18..+0x20) of a model with the 0x58-byte vertex array verts:
 * - unless [0x1017d14c] is set (then only a normal y +0x1c below [0x1012ba80] culls it), a polygon of 3 or more
 *   vertices facing away from the camera (the normal times the first vertex minus the camera 0x10185c48.., as a
 *   float, not negative) is skipped;
 * - each vertex not yet done this frame (+0x30 bit 4) is moved by minus the camera and turned by the matrix
 *   0x10185ba0 into +0x20..+0x28, marked done and given the near / far bits 1 / 2 ([0x10185c50] / [0x10185bf8]);
 *   all vertices beyond the same plane skip the polygon;
 * - the vertices are handed to the renderer callback [0x1014cb44] one by one (Sim_100a5c10 / Sim_100a5d25 for those
 *   in front of the near plane, Render_ClipEdgeNear / Sim_100a6009 for the edges that cross it), with the screen
 *   coordinates of the kinds 0xb000..0xe000 (3 or 4 vertices) from the tables 0x1014cd30 / 0x1014cd90 / 0x1014cdf0;
 * - with 3 or more vertices out ([0x101c9ea8]) and [0x101cc2a0] clear, a draw record (Sim_100a8943) gets the
 *   polygon, the depth of its vertex list 0x101c76a0 (mean, nearest or farthest by [0x101cc2b0], plus
 *   [0x1012ba84] with bit 0), the list copied to [0x101c6740] and the renderer's answer [0x1014cb48](poly, verts,
 *   kind word, depth); it is queued at [0x101cc2ac] while [0x1017d138] < [0x1017d17c].
 *
 * The original copies its never-written local [ebp-0x78] into the edge pointer [ebp-0x2c] before the table is
 * chosen; every path overwrites the edge pointer before reading it, so that leftover is never used (DECISIONS.md,
 * round 18) and the C has no such copy.
 */
void __cdecl Sim_100a65e7(uint8_t *poly, uint8_t *verts);

#endif
