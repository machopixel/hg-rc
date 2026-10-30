/*
 * Round 3 (letter t) functions of HGSIM.DLL in 0x1008xxxx. Plain C, original
 * cdecl convention. A "rect" is five dwords as in view/view.h (x0, y0, x1,
 * y1 at +4, +8, +0xc, +0x10), 16.16 fixed point.
 */
#ifndef HG_SIM_UNSORTED_1008_W3T_H
#define HG_SIM_UNSORTED_1008_W3T_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * Scales a rect to a 320 x 200 screen: x0 and x1 times 319, y0 and y1 times
 * 199 (16.16 products, rounded). Then y0 and y1 are multiplied by the float
 * v / 0.8333333 and truncated to ints. Returns rect.
 */
int32_t *__cdecl Sim_1008ce40(int32_t *rect, float v);

/*
 * 1 when the point (x, y) is within a circle in the middle of rect: the
 * distance is x - half the width, and y - half the height divided by the float
 * at +0x44 of the camera (pointer at 0x1014cae0) truncated; the squared distance
 * is compared with the square of (width / 2 - 1), the width being x1 - x0 + 1.
 */
int __cdecl Sim_1008e915(const int32_t *rect, int x, int y);

#endif
