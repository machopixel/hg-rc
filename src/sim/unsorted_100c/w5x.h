/*
 * Round 5, letter x: small functions at 0x100c6e87..0x100cbe68 that call helpers which are not
 * converted yet (called through their address, SIM_FN).
 */
#ifndef HG_SIM_UNSORTED_100C_W5X_H
#define HG_SIM_UNSORTED_100C_W5X_H

#include <stdint.h>
#include "../simmem.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Not converted: 0x100fd760 and 0x100fefe0 are library code taking one argument (a stream pointer and a heap block). */
#define SIM_CALL_100fd760 SIM_FN(int(__cdecl *)(uint32_t), 0x100fd760)
#define SIM_CALL_100fefe0 SIM_FN(void(__cdecl *)(void *), 0x100fefe0)
#define SIM_CALL_1002c8cb SIM_FN(void(__cdecl *)(uint32_t, uint32_t, uint32_t, uint32_t), 0x1002c8cb)
#define SIM_CALL_100ca670 SIM_FN(void(__cdecl *)(uint32_t, uint32_t, uint32_t), 0x100ca670)

/* When the dword at p is non-zero runs 0x100fd760 on it; then 0x100fefe0(p). */
void __cdecl Sim_100c6e87(void *p);
/* When the word at obj+4 is non-zero: [obj+0xf8] = [0x109e89d8] and 0x1002c8cb([obj+0xec], 0, 0, 0). */
void __cdecl Sim_100c81b5(void *obj);
/* 0x100ca670(record, value, 0x40008) with record = the pointer table entry (0x109e8b40) indexed by [0x1017b8c8]. */
void __cdecl Sim_100cbe68(uint32_t value);

#endif
