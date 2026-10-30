/*
 * Round 9, letter d: HUD text functions of HGSIM.DLL at 0x10063037, 0x10065384, 0x1006561a and the
 * two-way test 0x100690d4. Floating point: only conversions (int to double/float, double arithmetic of
 * int values, truncation through Render_Ftol), bit-exact.
 *
 * Callees that are not converted (or patched elsewhere) are called through their original address:
 *   0x100648a0 (record, kind)                       0x10064d05 (index, value, kind)
 *   0x100ba7d5 (name, key, 0) -> resource (the font) (patched by another hook file)
 *   0x100ecce0 (font) -> height
 *   0x100ecea6 / 0x100f1e10 / 0x100f2f01 (target 0x109e3e00, y, x, font, text 0x101658d8, 0x109b9920)
 */
#ifndef HG_SIM_UNSORTED_1006_W9D_H
#define HG_SIM_UNSORTED_1006_W9D_H

#include <stdint.h>
#include "../simmem.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

typedef void(__cdecl *W9dRecordKindFn)(uint32_t record, int kind);
typedef void(__cdecl *W9dIndexValueFn)(uint32_t index, uint32_t value, int kind);
typedef uint32_t(__cdecl *W9dShapeFn)(char *name, uint32_t key, uint32_t zero);
typedef int(__cdecl *W9dShapeHeightFn)(uint32_t shape);
typedef void(__cdecl *W9dDrawTextFn)(uint32_t target, int32_t y, int32_t x, uint32_t shape, char *font, uint32_t pal);

#define W9D_CALL_100648a0 SIM_FN(W9dRecordKindFn, 0x100648a0)
#define W9D_CALL_10064d05 SIM_FN(W9dIndexValueFn, 0x10064d05)
#define W9D_CALL_100ba7d5 SIM_FN(W9dShapeFn, 0x100ba7d5)
#define W9D_CALL_100ecce0 SIM_FN(W9dShapeHeightFn, 0x100ecce0)
#define W9D_CALL_100ecea6 SIM_FN(W9dDrawTextFn, 0x100ecea6)
#define W9D_CALL_100f1e10 SIM_FN(W9dDrawTextFn, 0x100f1e10)
#define W9D_CALL_100f2f01 SIM_FN(W9dDrawTextFn, 0x100f2f01)

/* ---- w9d_hud.c ---- */
/*
 * For the object *holder with target word +0x118 (0 or bit 0x1000 set: nothing): kind 0x200 calls
 * 0x100648a0(PTRTAB(index), Sim_10052be7(index)), kind 0x400 calls 0x10064d05(index, Sim_100cba68(),
 * Sim_10052c02(index)); kind 0x100 takes the next text of the ring at 0x109e857c (counter 0x10176954, modulo 7
 * with sign, then incremented), projects the point +0x104..+0x10c with Sim_100d2669 and, when visible, draws
 * the text there with Sim_10065216 and palette 7 (unit word 0x109a8664 + index * 0x54 is 0x20) or 5 of the
 * palettes at 0x10176960 (0x200 bytes, mode [0x1017b940]) or 0x10178b60 (0x100 bytes).
 */
void __cdecl Sim_10063037(void *holder);
/*
 * Draws the message text 0x101658d8 with the font named by the pointer [0x109b9900] (loaded through 0x100ba7d5
 * with the key [0x1014cec4]): byte 0x109b992e = 7, x = [0x109e3e10] - 3 * height, y = trunc((double)[0x109e3e18]
 * - width (Sim_100ccb0d) * double [0x1012b5e0]), one of the three text routines by [0x101668e8] / [0x1017b940],
 * byte 0x109b992e = 9 and Sim_100bb2b0([0x109b9900], key) releases the font. Nothing when there is no font.
 */
void __cdecl Sim_10065384(void);
/*
 * The blinking message: while [0x101658c0] is set adds [0x1015dc80] to [0x101658c4]; when that reaches
 * [0x101658c8] (signed) counts [0x101658cc] down: at 0 clears 0x101658c0, 0x101658cc and 0x101658d4, else
 * plays 0x10145048 (-1000, 0, 5, 10) when [0x101658d4] is set, toggles [0x101658d4] and resets [0x101658c4].
 * Then Sim_10065384 when [0x101658d4] is set.
 */
void __cdecl Sim_1006561a(void);
/* Sim_10068e77(a, b, x, y, z), and when that is 0 the same with a and b swapped. */
int __cdecl Sim_100690d4(uint8_t **a, uint8_t **b, float *x, float *y, float *z);

#endif
