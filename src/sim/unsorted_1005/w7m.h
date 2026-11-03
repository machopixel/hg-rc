/*
 * Round 7, letter m: functions at 0x10051a36..0x1005346f (a shaded bar drawn with lines, white-space trimming).
 * Helpers that are not converted yet (or that another hook file patches) are called through their original
 * address (SIM_FN) with the prototypes below.
 */
#ifndef HG_SIM_UNSORTED_1005_W7M_H
#define HG_SIM_UNSORTED_1005_W7M_H

#include <stdint.h>
#include "../simmem.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Patched by another hook file (0x100ef727): line drawing (surface, x0, y0, x1, y1, 0, colour). */
#define W7M_CALL_100ef727 SIM_FN(void(__cdecl *)(uint32_t, int32_t, int32_t, int32_t, int32_t, int32_t, uint32_t), 0x100ef727)
#define W7M_CALL_100e86a1 SIM_FN(void(__cdecl *)(uint32_t, int32_t, int32_t, int32_t, int32_t, int32_t, uint32_t), 0x100e86a1)

/*
 * Draws len vertical lines from (x + i, y) to (x + i, y - height) (nothing when height <= 0): the first quarter
 * of len (rounded towards zero, like the halves) in colour - 1, the second in colour, the third in colour - 1 and
 * the rest in colour - 2, each colour looked up in the word table at 0x101659e0.
 */
void __cdecl Sim_10051a36(uint32_t surface, int32_t x, int32_t y, int32_t len, int32_t height, int32_t colour);
/*
 * Removes white space (C library class 8) from both ends of s in place: skips the leading characters (returns the
 * pointer to the first other one) and ends the string after the last other one. Returns NULL for NULL.
 */
char *__cdecl Str_TrimSpace(char *s);

#endif
