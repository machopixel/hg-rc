/*
 * Projection, clipping and vertex-block code of the renderer-facing path
 * (wave 3B). These functions did their floating-point work on the x87 stack;
 * here it is done in double, so a result can differ from the original in the
 * last bits of a float (never in a truncation to int, see Render_Ftol).
 *
 * Vertex types used below (offsets proven by the code):
 *   scene vertex (0x58 bytes, array handed to Render_ClipPolygon):
 *     +0x0c,+0x10,+0x14 world x,y,z   +0x20,+0x24,+0x28 camera x,y,z (output)
 *     +0x30 byte: 0 inside, 1 nearer than the near plane, 2 beyond the far plane
 *   projected vertex (0x40 bytes, from the pool at 0x1017d174):
 *     +0x00,+0x04,+0x08 camera x,y,z   +0x18,+0x1c screen x,y (whole pixels)
 *     +0x28 clip bits (1 left, 2 right, 4 top, 8 bottom)   +0x29 bit 0 = projected
 *   plugin vertex (36 bytes): x, y, z, 1/z, int at +0x10 (light), r,g,b,a bytes at
 *     +0x14..+0x17, fog dword at +0x18 (top byte), u, v at +0x1c,+0x20
 * The plugin's Render (slot 0x109e3d30) is called through the slot like the
 * original; see w2m.h and docs/renderer.
 */
#ifndef HG_SIM_RENDER_W2N_H
#define HG_SIM_RENDER_W2N_H

#include <stddef.h>
#include <stdint.h>

#include "../simmem.h"
#include "w2m.h"

/*
 * The runtime's float-to-int truncation (0x100fcb08, x87 FISTP with round
 * toward zero, low 32 bits of the 64-bit result). Out of range values and NaN
 * give 0 there (the 64-bit "integer indefinite" is 0x8000000000000000).
 */
static inline int32_t Render_Ftol(double d)
{
    if (!(d >= -9223372036854775808.0 && d < 9223372036854775808.0))
        return 0;
    return (int32_t)(uint32_t)(uint64_t)(int64_t)d;
}

/*
 * Projects a scene-to-camera vertex: screen x = trunc(x * (K / z)) + [0x10185bd4],
 * y = [0x10185bf4] - trunc(y * (K / z)) (K = float at 0x1012ba78) into +0x18/+0x1c,
 * sets the clip bits and the "projected" flag once, and appends the vertex to the
 * list at 0x101c76a0 (count 0x101c9ea8; when full clears 0x101c9ea4). Returns its
 * argument. NOTE: hooks_view.c patches the original (sub-pixel table), so the
 * converted callers call it through the original address.
 */
void *__cdecl Render_ProjectVertex(unsigned char *vertex);

/*
 * Transforms `count` scene vertices (0x58 bytes) by the 3x3 matrix at 0x10185ba0
 * after subtracting the camera position (0x10185c48, 0x10185c44, 0x10185c4c),
 * classifies each against near/far (floats, passed as raw bits), clips every
 * edge that crosses a plane and projects the vertices that come out. Returns
 * the number of projected vertices in the list at 0x101c76a0, or 0 when
 * [0x101c9ea4] is 0 (list or pool overflow).
 */
unsigned __cdecl Render_ClipPolygon(unsigned char *verts, unsigned count, uint32_t nearBits, uint32_t farBits);

/* Fog byte: Fog function 0x10058ac2(z, arg) (1.0 near .. 0.0 far) times the float at 0x10143fcc, truncated. */
int __cdecl Render_FogLevel(float z, float arg);

/*
 * Pixels of texture record `index` (< 0x200) and its size; NULL when there is
 * none. Records 0..0xff / 0x100..0x1ff may be answered from the two cached
 * images named at 0x1012dbdc and 0x1012dbe8; otherwise the record of the table
 * at 0x10ac1b00 names an entry of the name table at 0x10a51ac0 whose image is
 * loaded on first use. The image starts with two 16-bit words (width, height);
 * the return value points behind them.
 */
void *__cdecl Render_GetTextureImage(int index, int16_t *width, int16_t *height);

/*
 * Finish the 36-byte plugin vertices of a polygon (1/z, colour bytes from
 * 0x10143fdc/e0/e4, light value) and pass them to Render with the texture of
 * `index` (+0x100 for the first one). They return 1, or 0 when there are
 * fewer than 3 vertices or no texture. Render ops: ..._10006609: 2;
 * ..._10006744: 1 if `flag`, else 6; ..._100068d9: 6.
 */
int __cdecl Render_SubmitVerts_10006609(int index, int count, unsigned char *verts);
int __cdecl Render_SubmitVerts_10006744(int index, int count, unsigned char *verts, int flag);
int __cdecl Render_SubmitVerts_100068d9(int index, int count, unsigned char *verts);

/*
 * Builds plugin vertices from `count` pointers to projected vertices and
 * passes them to Render (op 2 when [0x1017d0ac], else 6) with `texture`;
 * returns 0 without doing anything when texture is NULL.
 * The last two vertices get fog dword 0xff000000, the others 0 (kept as is).
 */
int __cdecl Render_SubmitVertexPtrs_100a0339(void *texture, void **projected, int count);

/*
 * Clips `count` scene vertices (Render_ClipPolygon, near/far from the camera
 * at [0x1014cae0] +0x3c/+0x40) and hands the result to the plugin (op 0xf,
 * vertices in reverse order, fog from Render_FogLevel).
 */
void __cdecl Render_SubmitClipped_100ac738(unsigned char *verts, unsigned count);

/* Like the previous one, for the vertices of model part `part` of object `obj` (fog parameter obj+0x18). */
void __cdecl Sim_100ad1f3(unsigned char *obj, unsigned char *part);

#endif
