/*
 * Round 8, letter x: functions at 0x10089e91..0x1008e9af. A scene object's placement in the 9 x 9 cell grid
 * (0x10089e91), the start-up screens of 0x1008b6a0 / 0x1008b78f and the HUD helpers that measure, frame and
 * underline text, tile a shape over a pane and turn angles or points into pane coordinates.
 * Helpers that are not converted yet (or that another hook file patches) are called through their original
 * address with SIM_FN and the prototypes below.
 */
#ifndef HG_SIM_UNSORTED_1008_W8X_H
#define HG_SIM_UNSORTED_1008_W8X_H

#include <stdint.h>
#include "../simmem.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Not converted: upper-cases its two texts in place and looks the first up as a shape resource (0 when absent). */
#define W8X8_CALL_100ba7d5 SIM_FN(void *(__cdecl *)(void *, void *, int), 0x100ba7d5)
/* Patched by hooks_hud.c / not converted: draw a shape (pane, shape, number, x, y). */
#define W8X8_CALL_100f0205 SIM_FN(int(__cdecl *)(void *, void *, uint32_t, uint32_t, uint32_t), 0x100f0205)
#define W8X8_CALL_100e91dc SIM_FN(int(__cdecl *)(void *, void *, uint32_t, uint32_t, uint32_t), 0x100e91dc)
/* Patched by hooks_hud.c / not converted: draw a line (pane, x0, y0, x1, y1, mode, colour). */
#define W8X8_CALL_100ef727 SIM_FN(int(__cdecl *)(void *, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t), 0x100ef727)
#define W8X8_CALL_100e86a1 SIM_FN(int(__cdecl *)(void *, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t), 0x100e86a1)
/* Not converted: the height of a font / the width of one character of it. */
#define W8X8_CALL_100ecce0 SIM_FN(int32_t(__cdecl *)(void *), 0x100ecce0)
#define W8X8_CALL_100eccf3 SIM_FN(int32_t(__cdecl *)(void *, uint32_t), 0x100eccf3)
/* Not converted: (width << 16) | height of shape `number` of a shape table. */
#define W8X8_CALL_100ed743 SIM_FN(uint32_t(__cdecl *)(void *, uint32_t), 0x100ed743)
/* Not converted: one pixel (pane, x, y, colour). */
#define W8X8_CALL_100e84f0 SIM_FN(int(__cdecl *)(void *, uint32_t, uint32_t, uint32_t), 0x100e84f0)
/* Patched by hooks_input.c: the next key (only ax is read). */
#define W8X8_CALL_10057f83 SIM_FN(int16_t(__cdecl *)(void), 0x10057f83)
/* Not converted: (5) gives non-zero while that condition holds. */
#define W8X8_CALL_100766ca SIM_FN(int(__cdecl *)(int), 0x100766ca)
/* Not converted (in the w8u list): loads a shape resource (a, b, c, 0). */
#define W8X8_CALL_100282d4 SIM_FN(void *(__cdecl *)(uint32_t, uint32_t, uint32_t, int), 0x100282d4)
/* Not converted: one step of the wait loop of Sim_1008b78f. */
#define W8X8_CALL_10055048 SIM_FN(void(__cdecl *)(void), 0x10055048)
/* Not converted: the C library fmod (result in st(0)). */
#define W8X8_CALL_100fcb30 SIM_FN(double(__cdecl *)(double, double), 0x100fcb30)
/* Not converted (in the w8w list): table lookups of one / two floats, result in st(0). */
#define W8X8_CALL_10084766 SIM_FN(double(__cdecl *)(float), 0x10084766)
#define W8X8_CALL_1008486e SIM_FN(double(__cdecl *)(float, float), 0x1008486e)
/* Not converted: pane drawing routines (pane or record, centre x, centre y, a, b, c). */
#define W8X8_CALL_100eb6e8 SIM_FN(int(__cdecl *)(const void *, int32_t, int32_t, int32_t, int32_t, uint32_t), 0x100eb6e8)
#define W8X8_CALL_100eba29 SIM_FN(int(__cdecl *)(const void *, int32_t, int32_t, int32_t, int32_t, uint32_t), 0x100eba29)
/* Not converted: (rect, centre pair, flags, float bits, out). */
#define W8X8_CALL_1008ec46 SIM_FN(void(__cdecl *)(const void *, int32_t *, uint32_t, uint32_t, int32_t *), 0x1008ec46)

/*
 * Files a scene object (flags word +0) in a cell list: +0x8c / +0x90 = 9999, linked (Sim_1008a8f5) behind the
 * list Sim_10088804 gives (when [0x1017c8c4] is set) or behind 0x1017b9d8 (flag 0x1000) / 0x1017b988; kind
 * +0x34 = 4 sets flag 0x800. Without flag 0x800: when [0x1017c8c4] is set the position (node +0x28 through
 * Sim_100a9cfb, else +0x44 / +0x4c) picks the cell trunc(trunc((p + offset) / size) + 0.5) of the 9 x 9 grid at
 * 0x1017ba28 (0x18c per column, 0x2c per row; outside it the cell [0x1017c8c8]); else the cell is 0x1017b988.
 * The object goes to the front of the cell's list +0x10 (prev +0xc, next +0x10) and List_InsertBefore(obj, cell).
 * Does nothing unless [0x1017c8cc] is 0x1017b988.
 */
void __cdecl Sim_10089e91(uint8_t *obj);

/*
 * The shape named by [0x109e85c4] (0x100ba7d5 with [0x1014cec0]): the first time ([0x1014b94c] set) the
 * rectangle 0x1014b938 is fitted to 0x109e3e20 (View_ScaleRect, Sim_1008d2d0); then drawn at (0, 0) in it
 * (0x100f0205 when [0x1017b940] is set, else 0x100e91dc). Returns the draw routine's answer (0 without shape).
 */
int __cdecl Sim_1008b6a0(void);
/*
 * Unless 0x100766ca(5) or [0x1015ecd0]: Sim_1008b8de, Sim_1000e983(0x1014b968, 0, 0, Sim_1000edce()), the
 * shape from 0x100282d4([0x1014cf8c], [0x101c64f0] + 0x74, [0x1014cec0], 0) shown as in Sim_1008b6a0 (and the
 * display driver's +0x18 when [0x1017b920] is set), then 0x10055048 and Sim_1001f454 until the word
 * 0x1015fd4c becomes non-zero, the word cleared, Sim_1000e983(0x1014b974, -5000, 0, Sim_1000edce()) and
 * Sim_1008b900.
 */
void __cdecl Sim_1008b78f(void);

/* A line under one text line: (pane, 0, y + font height, pane width - 1, same y). Returns the line's answer. */
int __cdecl Sim_1008d65d(void *pane, uint32_t unused, int32_t y, void *font, uint32_t colour);
/*
 * Underlines `text` at (x, y + font height) over the sum of its character widths (Sim_100f2cec when
 * [0x101668e8] is set, which also steps over two-byte characters, else 0x100eccf3). Returns the line's answer.
 */
int __cdecl Sim_1008d6f7(void *pane, const uint8_t *text, int32_t x, int32_t y, void *font, uint32_t colour);
/* A frame around `text` at (x, y): from (x, y - 1) to (x + width, y + font height + 1). Returns the last answer. */
int __cdecl Sim_1008d8c0(void *pane, const uint8_t *text, int32_t x, int32_t y, void *font, uint32_t colour);
/*
 * Measures a text of lines split by '\n': out +4 = +8 = 0, +0xc = (widest line << 16) / (0x10000 - [0x1014b9a8]),
 * +0x10 = (lines * font height << 16) / (0x10000 - [0x1014b9ac]). Returns out, 0 when an argument is 0.
 */
int32_t *__cdecl Sim_1008de0a(const char *text, void *font, int32_t *out);
/*
 * Shows a pixel of colour byte +4 at the point (x / 2 + 0x8000, 0x8000 - y / 2) of the pane a[0] (both truncated
 * first, scaled by Rect_ScalePoint), lets the display driver show it ([0x1017b920]: its +0x18) and reads a key.
 * Returns 0 for Escape (0x1b), else 1 (also when a or its pane is 0).
 */
int __cdecl Sim_1008df64(void *a, float x, float y);
/* Tiles shape `number` over the pane (as many as fit from (0, 0), rounded up). */
void __cdecl Sim_1008e0fe(void *pane, void *shape, uint32_t number);
/*
 * The angle fmod(fmod(angle, 360) + [0x1012b918], 360) as a point on the pane's frame (Sim_1008e489 with the
 * half size, quadrant bits from the angle and the slope -Sim_1008e211(sin(a + 90), sin(a) * camera +0x44)).
 * Returns out.
 */
int32_t *__cdecl Sim_1008e320(const int32_t *rect, float angle, int32_t *out);
/* 0x100eb6e8 / 0x100eba29 (rect, centre, n = half width - 1 scaled by f / [0x1012b92c], n * camera +0x44, c). */
int __cdecl Sim_1008e707(const int32_t *rect, uint32_t c, float f);
int __cdecl Sim_1008e7a1(const int32_t *rect, uint32_t c, float f);
/* 0x100eba29 for the rectangle rect moved by the box `box` (relative centre, half width - 1, scaled). */
int __cdecl Sim_1008e83b(const int32_t *rect, const int32_t *box, uint32_t c);
/* The direction from the pane centre to `pt` as 0x1008ec46(rect, centre, quadrant bits, slope, out). Returns out. */
int32_t *__cdecl Sim_1008e9af(const int32_t *rect, const int32_t *pt, int32_t *out);

#endif
