/*
 * Round 5, letter c: function at 0x100d255e (and the floating-point ones) that call helpers which are not converted yet.
 * Those helpers are called through their original address (SIM_FN) with the prototypes below.
 */
#ifndef HG_SIM_UNSORTED_100D_W5C_H
#define HG_SIM_UNSORTED_100D_W5C_H

#include <stdint.h>
#include "../simmem.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Not converted: fills a 48-byte block (twelve dwords) from six dwords. */
#define SIM_CALL_10060656 SIM_FN(void(__cdecl *)(uint32_t *, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t), 0x10060656)

/* Not converted: (object, three out floats). */
#define SIM_CALL_100d6f92 SIM_FN(void(__cdecl *)(void *, float *, float *, float *), 0x100d6f92)

/*
 * Builds the 12-dword block with 0x10060656 from the dwords at obj+0x10, +0xc, +0x14, +0, +4, +8, stores its
 * first nine in obj+0x54 through Mat3_Transpose and its last three dwords at obj+0x78, +0x7c, +0x80.
 */
void __cdecl Sim_100d255e(void *obj);

/*
 * Floating point (x87 at the 53-bit precision the Windows loader sets: intermediates are doubles, stored results floats).
 * Gets three floats a, b, c from 0x100d6f92(obj) and three (d, e, f) from Sim_100a9cfb(the pointer at obj+0x4c), adds the float that
 * [0x1015ef1c] points to (when set) to e, and calls Sim_1009cb40(out, d, e, f, a, b, c, 150000.0f).
 */
void __cdecl Sim_100d6f16(void *obj, void *out);
/*
 * Builds the 0x44-byte block with Sim_100d6f16, applies Sim_1009cd36 with the float of Sim_100d6e99(obj), gets three floats
 * from Sim_100d709d(obj, ..., q) and stores block[3..5] minus those in *x, *y, *z.
 */
void __cdecl Sim_100d6ea7(void *obj, float *x, float *y, float *z, void *q);

#endif
