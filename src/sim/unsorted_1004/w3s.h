/*
 * Wave 3S, original 0x10044420..0x1004f19d. Plain C, original cdecl convention.
 */
#ifndef HG_SIM_UNSORTED_1004_W3S_H
#define HG_SIM_UNSORTED_1004_W3S_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * Fills the record: +4 = a, +8 = b, +0xc = (float)(d * 181.0 [0x101115d8]), +0 = dword [0x1015dc78].
 * Returns 1 when the float at +0xc is above the float at 0x101115e0, else 0.
 */
int __cdecl Sim_10047b76(void *rec, uint32_t a, uint32_t b, double d);
/* Like Sim_10047b76, plus +0x10 = e, and the limit is the float at 0x101115e4. */
int __cdecl Sim_10047c92(void *rec, uint32_t a, uint32_t b, double d, uint32_t e);
/*
 * Sets up the eight records at 0x109b9bb0, 0x109b9a90, 0x109b9af0, 0x109b9ba0, 0x109b9b90,
 * 0x109b9ae0 (Sim_10047b76) and 0x109b9ac0, 0x109b9aa0 (Sim_10047c92, e = 360.0f) with a = b = 0 and
 * d = 0.5, 0.5, 0.7, 1.0, 0.2, 0.2, 0.2, 1.0; stores 1.0f in 0x1015fdb4; clears the seven dwords of each
 * of the five 0x1c-byte records at 0x109b9b00; when the pointer-table entry selected by [0x1017b8c8] is
 * non-zero copies it to 0x1015ef14 and the selector to 0x1015ef18; then Sim_10043f10 and
 * Sim_100448cd([0x1015ef00]).
 */
void __cdecl Sim_10044420(void);
/* p points to three floats: (float)(Sim_1002db2d(p[0], p[1], p[2]) + 200.0f [0x1011157c]), returned in st(0). */
float __cdecl Sim_1004487c(const float *p);
/* out[i] = camera[i] - obj[0x58 + 4i] for i = 0..2, the camera being the structure [0x1014cae0] points to; returns out. */
float *__cdecl Sim_1004f19d(const void *obj, float *out);

#endif
