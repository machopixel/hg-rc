/*
 * Round 10: game functions of HGSIM.DLL around 0x10080000..0x1008ffff, converted one at a time.
 * Calls of functions that are not converted yet go through their original address (SIM_FN).
 */
#ifndef HG_SIM_UNSORTED_1008_W10_H
#define HG_SIM_UNSORTED_1008_W10_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Writes a point moved from pt (two ints) into out and returns out. h = ((rect+0xc) - (rect+4) + 1) / 2 - 2,
 * v = trunc((h + 1) * camera+0x44) - 1 (camera = [0x1014cae0]). Modes 0..3: x - trunc(Sim_10084766(angle + 90)
 * * h), y - trunc(Sim_10084766(angle) * v); 5: (x, y + v); 7: (x, y - v); any other: (0, 0). */
int32_t *__cdecl Sim_1008ec46(const void *rect, const int32_t *pt, uint32_t mode, float angle, int32_t *out);

/* a = fmod(fmod(angle, 360) + 360, 360) (C runtime fmod 0x100fcb30), the centre of rect ((right - left + 1) >> 1,
 * (bottom - top + 1) >> 1) and the mode bits: 1 when 90 <= a <= 270, 2 when not a > 180, 4 when not
 * |sin(a + 90)| >= 1e-7 (Sim_10084766, fabs 0x100fcb70). Returns Sim_1008ec46(rect, centre, mode,
 * Sim_1008486e(sin(a) * camera+0x44, -sin(a + 90)), out). */
int32_t *__cdecl Sim_1008eaaf(const void *rect, float angle, int32_t *out);

/* Draws text into pane with font, wrapped at the pane's width: nothing when one of them is 0 or the text is
 * empty. A heap copy (HeapAlloc / HeapFree on [0x1017b8f8]) is cut into lines at newlines, or at the last space
 * or tab before the character that passes the width (character widths from 0x100f2cec / 0x100eccf3 by the display
 * mode; without a blank the overflowing character itself is cut off), starting at Rect_ScalePoint(pane, the point
 * 0x1014b9a8) and moving down by the font's height (0x100ecce0) while above the pane's height; each non-empty
 * line is drawn with the routine of the display mode. */
void __cdecl Sim_1008dba2(const int32_t *pane, const char *text, void *font);

#endif
