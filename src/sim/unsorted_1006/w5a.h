/*
 * Round 5, letter a: functions of 0x10060882..0x1006ab98 that call helpers which are not converted yet.
 * Those helpers are called through their address (SIM_FN) with the prototypes below. The floating point
 * follows the conventions of util/w3o.h: every intermediate is a double, and a float is stored where the
 * original stored one.
 */
#ifndef HG_SIM_UNSORTED_1006_W5A_H
#define HG_SIM_UNSORTED_1006_W5A_H

#include <stdint.h>
#include "../simmem.h"
#include "../libm/hg_math.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Not converted: the C library's square root (a double on the stack, the result in st(0)). */
#define W5A_SQRT hg_sqrt
/* Not converted: the C library's absolute value of a double. */
#define W5A_FABS hg_fabs
/* Not converted: the C library's floating point remainder fmod(x, y). */
#define W5A_FMOD hg_fmod
/* Not converted: a lookup in the sine table at 0x103719a0 (float in, result in st(0)). */
#define W5A_SINTAB SIM_FN(double(__cdecl *)(float), 0x10084766)
/* Not converted: the C library's assertion handler (expression, file, line). */
#define W5A_ASSERT SIM_FN(void(__cdecl *)(const char *, const char *, uint32_t), 0x100ff860)
/* Not converted: (a, b, i, 0, 1). */
#define W5A_CALL_1006b52f SIM_FN(int(__cdecl *)(uint32_t, void *, int, uint32_t, uint32_t), 0x1006b52f)

/* Normalizes the vector (*x, *y, *z) in place: each component times 1.0 / sqrt(x*x + y*y + z*z). */
void __cdecl Sim_10060882(float *x, float *y, float *z);
/*
 * With s = sine-table(ang / 2.0) and k = sine-table(ang / 2.0 + 90.0): out[0..2] = s * (a, b, c) and
 * out[3] = k. Asserts (expression "out", file MATRIX.C, line 0x2a2) first when out is null.
 */
void __cdecl Sim_100610ae(float a, float b, float c, float ang, float *out);
/*
 * Does nothing when any output pointer is null or fabs(ang) is below the float at 0x1012b5f8 (or NaN).
 * Otherwise with s = sine-table(ang + 90.0) and k = sine-table(ang): *p1 = a * s + c * k, *p2 = the dword
 * bits, *p3 = c * s - a * k.
 */
void __cdecl Sim_10068c48(float a, uint32_t bits, float c, float ang, float *p1, uint32_t *p2, float *p3);
/*
 * Unless the dword obj + 0x50c is 1: the float at [obj + 0] + 0x110 = fmod(fmod(f1 + f2, 360.0) + 360.0, 360.0)
 * with f1 the float at [obj + 0] + 0x68 and f2 the float at obj + 0xc.
 */
void __cdecl Sim_10069f68(void *obj);
/*
 * With n = the word at (dword [b + 0x88]) + 0x3c * (i - 1) + 0x34: calls 0x1006b52f(a, b, i, 0, 1) n times.
 */
void __cdecl Sim_1006ab98(uint32_t a, void *b, int i);

#endif
