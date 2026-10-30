/*
 * Round 5 (letter y) functions of HGSIM.DLL in 0x100axxxx. Plain C, original cdecl convention.
 * They call helpers that are not converted yet through their original address (SIM_FN), so the
 * original code still runs for those. Names carry the original address because the code alone
 * does not say what the data is for.
 */
#ifndef HG_SIM_UNSORTED_100A_W5Y_H
#define HG_SIM_UNSORTED_100A_W5Y_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * handle = the dword at 0x101c65e0 + i * 0x14a; calls Sim_10105fe0(handle, b, 0) and returns
 * Sim_100a796c(handle, a, d).
 */
uint32_t __cdecl Sim_100a81fb(uint32_t a, int i, uint32_t b, uint32_t d);
/* Stores Sim_1010c9f0(text) in the dword at obj+0x90. */
void __cdecl Sim_100a9b8d(void *obj, const char *text);
/* Calls Sim_1008a1ea(obj[0x8c]) when that field is set, then does the same for every node of the child chain (first child obj+4, next sibling +8). */
void __cdecl Sim_100aa4a2(void *obj);
/* Stores the low 16 bits of value at +2 of obj[0x8c] (when set) and does the same for the whole child chain. */
void __cdecl Sim_100aa4f0(void *obj, int value);
/* Calls Sim_100aa742(obj); when the dword at 0x1017c8c4 is set and obj[0] is 0, Sim_10088a2d(obj[0x8c]). */
void __cdecl Sim_100aa799(void *obj);
/*
 * Sim_10060d2e(src, obj + 0x7c), clears bit 15 of obj[0x94] and copies the dwords src[0x24..0x2c] to obj[0x70..0x78].
 */
void __cdecl Sim_100aa87a(void *obj, const void *src);
/* When a is not NULL calls Sim_100ab0d6(a[4], b); then *p4 = dword 0x101c64f4 and *p3 = dword 0x101c6504. */
void __cdecl Sim_100ab151(void *a, uint32_t b, uint32_t *p3, uint32_t *p4);

#endif
