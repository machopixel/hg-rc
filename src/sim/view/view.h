/*
 * Converted functions of HGSIM.DLL around the view: the display size the
 * game draws into, rectangle and point maths in 16.16 fixed point against
 * that size, 3x4 matrices, and the small object lists the view code walks.
 * Plain C with no knowledge of the hook layer; every function keeps the
 * original cdecl convention.
 *
 * The display size: when the display is set up the DLL stores the screen
 * width - 1 at 0x109e3e14 and the height - 1 at 0x109e3de0 (written from
 * 0x1017b918 / 0x1017b91c next to the display surface description). The
 * rectangle functions below scale by those two values.
 *
 * A "rect" is five dwords: field 0 is not touched by any function here, then
 * x0, y0, x1, y1 at offsets 4, 8, 0xC, 0x10. Coordinates are 16.16 fixed point
 * (one = 0x10000), the products round to nearest.
 */
#ifndef HG_SIM_VIEW_H
#define HG_SIM_VIEW_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* ---- viewrect.c ---- */
/* out = rect with x0/x1 scaled by the display width - 1 and y0/y1 by the height - 1. Returns out. */
int32_t *__cdecl View_ScaleRect(void *unused, const int32_t *rect, int32_t *out);
/* out[0] = point[0] * (width - 1), out[1] = point[1] * (height - 1). Returns out. */
int32_t *__cdecl View_ScalePoint(void *unused, const int32_t *point, int32_t *out);
/* out = the rect moved to the middle of the display (its size is kept). Returns out. */
int32_t *__cdecl View_CenterRect(void *unused, const int32_t *rect, int32_t *out);
/* out[0] = point[0] * width of area, out[1] = point[1] * height of area. Returns out. */
int32_t *__cdecl Rect_ScalePoint(const int32_t *area, const int32_t *point, int32_t *out);
/* out = rect scaled to the size of area and moved to its origin. Returns out. */
int32_t *__cdecl Rect_ScaleToRect(const int32_t *area, const int32_t *rect, int32_t *out);
/* out = a + (b - a) * t on the x pair (skipped if skip == 2) and the y pair (skipped if skip == 1). Returns out. */
int32_t *__cdecl Rect_Interpolate(const int32_t *a, const int32_t *b, int32_t *out, int32_t t, int32_t skip);

/* ---- matrix.c ---- */
/* Copies the 3x3 part of a 3x4 float matrix transposed. */
void __cdecl Mat3_Transpose(const uint32_t *src, uint32_t *dst);
/* 3x3 identity with a zero fourth row (12 floats). */
void __cdecl Mat3x4_SetIdentity(uint32_t *m);

/* ---- objlist.c ---- */
/* Links node `node` into a doubly linked list in front of `before` (next at +0x14, prev at +0x18), unless it is already linked or flagged. */
void __cdecl List_InsertBefore(void *node, void *before);
/* Unlinks a node and clears its two links. */
void __cdecl ObjList_Unlink(void *node);

int32_t __cdecl Sim_10072ff2(void);
void *__cdecl Sim_1007652f(int32_t key);
int32_t __cdecl Sim_10076624(void *stack, int32_t value);
void __cdecl Sim_10077fd8(void *obj);
void __cdecl Sim_10078177(void);
int32_t __cdecl Sim_10079e3b(const uint8_t *id);
int32_t __cdecl Sim_10079feb(int32_t kind);
int32_t __cdecl Bits_Count(uint32_t value);
void __cdecl Sim_10087c86(void);
void __cdecl Sim_1008f630(void **pp);

#endif
