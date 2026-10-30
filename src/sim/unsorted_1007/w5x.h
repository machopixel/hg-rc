/*
 * Round 5, letter x: small functions around 0x10072d16..0x100788fc that call helpers which are not
 * converted yet (called through their address, SIM_FN).
 */
#ifndef HG_SIM_UNSORTED_1007_W5X_H
#define HG_SIM_UNSORTED_1007_W5X_H

#include <stdint.h>
#include "../simmem.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Not converted. */
#define SIM_CALL_1002c1f9 SIM_FN(void(__cdecl *)(void *), 0x1002c1f9)
#define SIM_CALL_1002c63a SIM_FN(void(__cdecl *)(void), 0x1002c63a)
#define SIM_CALL_1008d4f1 SIM_FN(void(__cdecl *)(uint32_t, uint32_t), 0x1008d4f1)
#define SIM_CALL_100652e8 SIM_FN(void(__cdecl *)(int, int, uint32_t, uint32_t), 0x100652e8)

/* When p is non-NULL runs 0x1002c1f9(p). */
void __cdecl Sim_10072d16(void *p);
/*
 * With p non-NULL and e = the dword at p: runs 0x1002c63a() when the dword at e+4 equals [0x1017b8c8]; then
 * Sim_100ac58e(dword at e+0x154) and sets that dword to -1.
 */
void __cdecl Sim_10072d3a(void *p);
/* With e = the dword at obj+0xec: 0x1008d4f1(e, a), then 0x100652e8(word at obj+0x100 >> 1, 2, b, e). */
void __cdecl Sim_100788fc(void *obj, uint32_t a, uint32_t b);

#endif
