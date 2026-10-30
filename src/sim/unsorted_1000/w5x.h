/*
 * Round 5, letter x: small functions around 0x10006530..0x1000ed52 that call helpers which are not
 * converted yet. Those helpers are called through their address (SIM_FN), with the prototypes below.
 */
#ifndef HG_SIM_UNSORTED_1000_W5X_H
#define HG_SIM_UNSORTED_1000_W5X_H

#include <stdint.h>
#include "../simmem.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Not converted: both take (name pointer, dword). */
#define SIM_CALL_100bb2b0 SIM_FN(int(__cdecl *)(const void *, uint32_t), 0x100bb2b0)
#define SIM_CALL_100b9203 SIM_FN(int(__cdecl *)(const void *, uint32_t), 0x100b9203)
/* Not converted: twelve dwords (the third is the name index from Sim_1000df9c). */
#define SIM_CALL_1000e737 SIM_FN(int(__cdecl *)(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t), 0x1000e737)
/* Not converted: seven dwords. */
#define SIM_CALL_1000eb12 SIM_FN(int(__cdecl *)(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t), 0x1000eb12)

/*
 * When the dword [0x1015cff0] is non-zero: calls 0x100bb2b0 and 0x100b9203 with the name at 0x10a3ae80 and
 * the dword [0x1014cf0c], then clears [0x1015cff0].
 */
void __cdecl Sim_10006530(void);
/* Sim_1000df9c(name), then 0x1000e737(0, 0, index, 0, a, [0x101321b4], b, c, -1, 0x450, 0, 0). */
void __cdecl Sim_1000e983(const char *name, uint32_t a, uint32_t b, uint32_t c);
/* 0x1000eb12(a0, a1, a2, a3, a4, 0x32, a5). */
void __cdecl Sim_1000eaeb(uint32_t a0, uint32_t a1, uint32_t a2, uint32_t a3, uint32_t a4, uint32_t a5);
/*
 * Sim_1000df9c(name), then 0x1000e737(0, 0, index, 0, a, [0x101321b4], b, table[which], -1, flags | 0x2000, 0, 0)
 * with the dword table at 0x1012df30.
 */
void __cdecl Sim_1000ec60(const char *name, uint32_t a, uint32_t b, int which, uint32_t flags);
/* Sim_1000eaeb with the same six arguments. */
void __cdecl Sim_1000ed52(uint32_t a0, uint32_t a1, uint32_t a2, uint32_t a3, uint32_t a4, uint32_t a5);

#endif
