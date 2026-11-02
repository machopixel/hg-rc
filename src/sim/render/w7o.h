/*
 * Round 7, letter o: renderer-facing functions. A quad for a point (0x10089907), the particles of two kinds of
 * effect objects as screen quads (0x10094d7f, 0x10095453), a model drawn with the level of detail picked by
 * distance (0x100a4510) and the two edge clippers that Render_ClipPolygon calls through its thunks
 * (0x100a5d38 near plane, 0x100a6022 far plane).
 *
 * These only draw. They compute in double and store floats where the original does (see util/w3o.h); the
 * checker found them bit-exact, but as drawing code they may also be checked with its float tolerance.
 * The display driver's polygon routine is called through its slot 0x1014cb4c as (count, vertices, flags) with
 * 36-byte plugin vertices (render/w2n.h); vertex fields the original leaves unset stay unset.
 */
#ifndef HG_SIM_RENDER_W7O_H
#define HG_SIM_RENDER_W7O_H

#include <stdint.h>
#include "../simmem.h"
#include "../libm/hg_math.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Not converted: projects the point (*x, *y, *z) in place, returns non-zero when it is on the screen. */
#define W7O_CALL_100d2669 SIM_FN(int(__cdecl *)(float *, float *, float *), 0x100d2669)
/* Draws one polygon record (0x28 bytes) of a model with the model's vertex array (Sim_100a65e7, round 18, called
 * through its address). */
#define W7O_CALL_100a65e7 SIM_FN(void(__cdecl *)(void *, void *), 0x100a65e7)
/* Not converted: C library function of a double, result in st(0). */
#define W7O_CALL_100fcb70_R hg_fabs

/*
 * The square (x - s .. x + s) by (z - s .. z + s) at height y as four scene vertices, clipped and projected by
 * Render_ClipPolygon(near = camera [0x1014cae0] +0x3c, far = 1e6); with at least 3 vertices out, sends them to
 * the display driver (screen x, y, depth, 1.0, 0, 0xff, -, 1.0, 1.0) with flags | 0xa000.
 */
void __cdecl Sim_10089907(float x, uint32_t y, float z, float s, uint32_t flags);

/*
 * For each of the 16 particles of the effect: position + node position (Sim_100a9cfb), projected by 0x100d2669;
 * when visible a quad (x, y) .. (x + c, y + c), c = float at 0x1012b970, to the display driver with colour
 * +0x566. Returns 0 for a null effect, else 1.
 */
int __cdecl Sim_10094d7f(uint8_t *obj);
/* As Sim_10094d7f with the particle positions alone and the quad size the integer +0x56a. */
int __cdecl Sim_10095453(uint8_t *obj);

/*
 * Model +0x2c is a chain (next +0xc) of level-of-detail records (float distance +0); the record used is the
 * first when its distance equals the float at 0x1012ba68, else the last one before the first whose distance
 * times the camera's +0xb8 exceeds `range`. Stores it at +0x30, refreshes the model (Sim_100aa307) when +0x9c
 * differs from the record's +0x10, clears +0x2c and bit 2 of +0x30 of each of its vertices (count word +4,
 * 0x58 bytes from +0x18), sets the view matrix (node +0x28 +0x4c, or 0x1014ccf0), and draws its polygons
 * (count word +6, from record + dword +8, 0x28 bytes each) with 0x100a65e7 while [0x101c9ea4] stays set, with
 * [0x101cc2b0] = flags & 0x10f and [0x1017d12c] = +0xb0 (both cleared after, and +0xb0). Returns 1 when there
 * is nothing to draw, else 0.
 */
int __cdecl Sim_100a4510(void *obj, float range);

/*
 * Edge clippers: the projected vertex (pool, Sim_100a88ff) where the edge between two vertices meets the plane
 * z = [0x10185c50] (near, the vertices from Sim_100a5c10(obj, screen)) or z = [0x10185bf8] (far, from
 * Sim_100a5d25); position and the other interpolated fields of the first vertex moved along the edge unless
 * the z difference is below the float at 0x1012ba74.
 */
void *__cdecl Render_ClipEdgeNear(void *a, const uint32_t *screenA, void *b, const uint32_t *screenB);
void *__cdecl Render_ClipEdgeFar(void *a, void *b);

#endif
