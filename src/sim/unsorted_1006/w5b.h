/*
 * Round 5, letter b: functions around 0x10061d67..0x1006c9a0 that call helpers which are not converted yet
 * (called through their address with SIM_FN).
 *
 * Floating point: the original keeps intermediates in x87 registers with the 53-bit precision control the
 * Windows loader sets, so every intermediate is a double here and only the stored results are floats.
 */
#ifndef HG_SIM_UNSORTED_1006_W5B_H
#define HG_SIM_UNSORTED_1006_W5B_H

#include <stdint.h>
#include "../simmem.h"
#include "../libm/hg_math.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Not converted. */
#define SIM_CALL_10028b90 SIM_FN(int(__cdecl *)(uint32_t, int), 0x10028b90)
#define SIM_CALL_10062386 SIM_FN(void(__cdecl *)(void), 0x10062386)
/* Takes a float, returns a double in st(0); its uses in 0x100631ef and 0x10068bb2 are the two factors s and c of a rotation. */
#define SIM_CALL_10084766 SIM_FN(double(__cdecl *)(float), 0x10084766)
/* Takes a double on the stack, returns its absolute value in st(0). */
#define SIM_FABS hg_fabs
#define SIM_CALL_1006a12d SIM_FN(void(__cdecl *)(uint32_t, void *), 0x1006a12d)

/*
 * With rec = obj[0]: Sim_100aa39f(rec[0x48]); then for the three values obj[0x490], obj[0x48c] and (read again
 * after the first call) rec[0x48]: Sim_10028c6f(0x10028b90(value, 1)).
 */
void __cdecl Sim_10061d67(void *obj);
/*
 * Rect_ScalePoint(rect, p, p) with rect = the dword at +0xec of the objects [0x109e893c] and [0x109e8940] and p =
 * 0x10144ec8 and 0x10144ed0 (both points are scaled in place); float [0x10144f38] = (float)[0x109e3e1c] / [0x1012b5a0] /
 * [0x1012b5a4]; [0x10144f4c] = -1; [0x1017694c] = 0x10176938; 0x10062386().
 */
void __cdecl Sim_10062a3a(void);
/*
 * Rotation of the point (x, y) with two calls of 0x10084766: s = it(angle), c = it(angle + [0x1012b5a8]) (floats):
 * *outA = (int)(x * c - y * s), *outB = (int)(x * s + y * c), both truncated like the C library _ftol (low 32 bits).
 * For angle 0: *outA = x, *outB = y.
 */
void __cdecl Sim_100631ef(int *outA, int *outB, int x, int y, int angle);
/*
 * The same rotation for the (y, z) part of a vector, in floats: nothing at all unless outA, outB and outC are not NULL and
 * |angle| is at least [0x1012b5f8] (fabs through 0x100fcb70). *outA = x (the 32 bits, unchanged), *outB = y * c - z * s,
 * *outC = y * s + z * c with s = 0x10084766(angle) and c = 0x10084766(angle + [0x1012b650]) as floats.
 */
void __cdecl Sim_10068bb2(uint32_t x, float y, float z, float angle, uint32_t *outA, float *outB, float *outC);
/*
 * Unless dword obj[0x4f0] is 4 or 5: when flag is not 0, obj[0]'s dword +4 equals [0x1017b8c8] and [0x1015ebbc] is 0,
 * obj[0x4f0] becomes 5. Then 0x1006a12d(that dword +4, obj).
 */
void __cdecl Sim_1006c9a0(void *obj, int flag);

#endif
