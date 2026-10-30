/*
 * Wave 2 (letter i) additions to unsorted_1005: layout setup around 0x10050600..0x10050e80 and a constant.
 * Plain C, original cdecl convention. Names carry the original address
 * because the code alone does not say what the data is for.
 */
#ifndef HG_SIM_UNSORTED_1005_W2I_H
#define HG_SIM_UNSORTED_1005_W2I_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * Like the functions of fns.h: stores constant points in the structure and scales
 * them with Rect_ScalePoint by the rect whose pointer is at +0xec. Points at
 * +0xb4, +0xbc, +0xc4 and +0xcc are scaled in place; +0xc0, +0xc8 and +0xd0 are
 * taken from the dwords 0x109e8780, 0x109e8788 and 0x109e879c. Four points are
 * then derived for the dwords from 0x10151d68 (areaY0 + the scaled values), and
 * the byte table at 0x1015e3c0[256] is filled: i - (i & 0xf), and for i >= 0xe0
 * i - (i & 0x1f) when that low part is <= 0xf, else i - 0xf.
 */
void __cdecl Sim_10050671(void *obj);
/*
 * Runs the layout functions of fns.h over the structures stored at the
 * dwords 0x109e8918 .. 0x109e8954 (see the .c file for the order).
 */
void __cdecl Sim_10050c5b(void);
int __cdecl Sim_10057900(void); /* returns 1 */

#endif
