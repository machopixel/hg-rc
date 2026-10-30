/*
 * Wave 2 (letter i) additions to unsorted_1008: an empty function and a rect scaling around 0x10083000..0x1008e000.
 * Plain C, original cdecl convention. Names carry the original address
 * because the code alone does not say what the data is for.
 */
#ifndef HG_SIM_UNSORTED_1008_W2I_H
#define HG_SIM_UNSORTED_1008_W2I_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

void __cdecl Sim_10083495(void); /* does nothing */
/*
 * rect: five dwords as in view.h (x0, y0, x1, y1 at +4, +8, +0xc, +0x10). out =
 * the rect scaled by sx (x) and sy (y), 16.16 fixed point rounded, about its
 * middle point ((x1 - x0 + 1) >> 1 + x0, same for y), which stays in place.
 * Returns out.
 */
int32_t *__cdecl Rect_ScaleAroundCenter(const int32_t *rect, int32_t *out, int32_t sx, int32_t sy);

#endif
