/*
 * Wave 2g: leaf functions of HGSIM.DLL from 0x100289a2..0x1002b92d: a record
 * of rectangle fields, a table of 0x50-byte records at 0x109e9ba0 and the
 * rescaling of the rectangles at 0x109e8660. Names carry the original address
 * where the code alone does not say what the objects are.
 */
#ifndef HG_SIM_W2G_1002_H
#define HG_SIM_W2G_1002_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * dst[0xec] = src (the pointer itself); src is a rect as in view.h (x0, y0,
 * x1, y1 at +4, +8, +0xc, +0x10): dst words at +0xfc = x0, +0xfe = y0,
 * +0x100 = x1 - x0 + 1, +0x102 = y1 - y0 + 1.
 */
void __cdecl Sim_100289a2(void *dst, const void *src);
/* The word at obj + 4 = 1. */
void __cdecl Sim_10028a84(void *obj);
/* Resets record `index` of the table of 0x50-byte records at 0x109e9ba0 (words at +0 and +6 = 0, +8 = -1, dwords +0xc and +0x10 = 0, byte +0x18 = 0, dword +0x4c = 0x80000000). */
void __cdecl Sim_1002a16b(int index);
/* Sim_1002a16b(i) for i = 0..253. */
void __cdecl Sim_1002a1c1(void);
/*
 * For each of the 30 rects (0x14 bytes each, from 0x109e8660): rescales it in
 * place (Sim_1008cf1f, then View_ScaleRect), and the point at 0x109e8240 + 8 * i
 * in place against it (Rect_ScalePoint). When the pointer q at 0x10138158 + 4 * i
 * is non-null, block = q[1] holds three rect pointers at +4, +8 and +0xc: the
 * first two are rescaled into the rect (Rect_ScaleToRect, in place) and all
 * three get the address 0x109e3e20 stored in their field 0.
 */
void __cdecl Sim_1002b92d(void);

#endif
