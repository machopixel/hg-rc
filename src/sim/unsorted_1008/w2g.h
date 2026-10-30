/*
 * Wave 2g: 0x1008cf1f of HGSIM.DLL, a rect conversion next to the rect
 * scaling functions of view.h (same rect layout, 16.16 fixed point).
 */
#ifndef HG_SIM_W2G_1008_H
#define HG_SIM_W2G_1008_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * dst[1..4] = (src[1..4] << 16) / d with a signed 64-bit dividend, where d is
 * 319 for the fields +4 and +0xc and 199 for +8 and +0x10. Field 0 of dst is
 * not touched. Returns dst. The
 * original faults (divide overflow) when a quotient does not fit in 32 bits,
 * i.e. for |x| >= 319 * 32768 or |y| >= 199 * 32768; the C version then wraps.
 */
int32_t *__cdecl Sim_1008cf1f(const int32_t *src, int32_t *dst);

#endif
