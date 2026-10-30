/*
 * Round 3 (letter q) functions of HGSIM.DLL around 0x10098000..0x1009d000.
 * Names carry the original address because the code alone does not say what
 * the fields mean. Floating point is done as the original's x87 did it
 * (see util/w3q.h).
 */
#ifndef HG_SIM_UNSORTED_1009_W3Q_H
#define HG_SIM_UNSORTED_1009_W3Q_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * Resets 16 slots: dwords 0x101d0980[i] = -1, 0x101d0940[i] = i and the 0x40-byte records at 0x101d0440
 * (+0 = i, +0x10, +0x3c = -1, +4, +8, +0xc, +0x1c..+0x30 = 0). Then looks for the first i below
 * [0x1015dc68] whose dword 0x109e9bac + 0x50 * i has bit 0 clear (stores i in [0x1014bd8c]); with the
 * loop counter one past it (the original keeps using the counter, so the record after the match, or the
 * count when none), [0x1014bd90] = dword 0x109e9ba8 + 0x50 * counter. Sim_100b53f0(that, &a, &b, &c) gives
 * three floats; the object at [0x1014bd88] gets +0x18 = 1, +0x1c/+0x20/+0x24 = a, b, c minus and
 * +0x28/+0x2c/+0x30 = a, b, c plus the float at 0x1012b9ac, +0x34/+0x38/+0x3c = 0 and +0x10 = -1.
 */
void __cdecl Sim_10098be0(void);
/*
 * n = Render_Ftol(((int)obj[0x18] - Render_Ftol(f) - [0x1017cc58]) / (int)[0x1014bf00] * float[0x1012b9d8]),
 * limited to 0..15 (the division is done in double).
 */
int __cdecl Sim_1009b876(const void *obj, float f);
/*
 * Fills a record: floats +0, +4, +8 = a1, a2, a3; +0x24, +0x28, +0x2c = a4, a5, a6; +0x18, +0x1c, +0x20 =
 * scale * a4, a5, a6; +0xc, +0x10, +0x14 = a1, a2, a3 plus those products; +0x30 = scale; dword +0x34 = 1.
 */
void __cdecl Sim_1009cb40(void *obj, float a1, float a2, float a3, float a4, float a5, float a6, float scale);

#endif
