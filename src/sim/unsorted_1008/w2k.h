/*
 * Wave 2 (letter k) additions to unsorted_1008: leaf functions around
 * 0x100877a5..0x1008cd0a of HGSIM.DLL. Names carry the original address
 * because the code alone does not say what the data means.
 */
#ifndef HG_SIM_UNSORTED_1008_W2K_H
#define HG_SIM_UNSORTED_1008_W2K_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Clears the dword the pointer at 0x1015e698 points to. */
void __cdecl Sim_100877a5(void);
/* Returns 0. */
int __cdecl Sim_100877bc(void);
/* Stores the dword at 0x1015e9a0 in the dword the pointer at 0x1015e698 points to; returns 0. */
int __cdecl Sim_1008433f(void);
/*
 * 16.16 fixed-point scaling of a rectangle's size. With w = rect[3] - rect[1] and
 * h = rect[4] - rect[2] (dwords) and the dwords m[0..3]: out[0] = w * m[0],
 * out[1] = h * m[1], out[2] = w * m[2], out[3] = h * m[3], each product shifted
 * right by 16 and rounded to nearest (bit 15 is added). Returns out.
 */
int32_t *__cdecl Sim_1008cd0a(const int32_t *rect, const int32_t *m, int32_t *out);

#endif
