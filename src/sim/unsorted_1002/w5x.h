/*
 * Round 5, letter x: small functions around 0x1002009d..0x1002d3ca that call helpers which are not
 * converted yet (called through their address, SIM_FN).
 */
#ifndef HG_SIM_UNSORTED_1002_W5X_H
#define HG_SIM_UNSORTED_1002_W5X_H

#include <stdint.h>
#include "../simmem.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Not converted. */
#define SIM_CALL_1001f790 SIM_FN(int(__cdecl *)(void *, void *, int), 0x1001f790)
#define SIM_CALL_100b9203 SIM_FN(int(__cdecl *)(const void *, uint32_t), 0x100b9203)
#define SIM_CALL_10027fb8 SIM_FN(void(__cdecl *)(void *), 0x10027fb8)
#define SIM_CALL_10031ce0 SIM_FN(uint32_t(__cdecl *)(uint32_t, uint32_t, uint32_t), 0x10031ce0)
#define SIM_CALL_10031f95 SIM_FN(uint32_t(__cdecl *)(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t), 0x10031f95)

/* For 0 <= index < 3 (signed): 0x1001f790(record, 0, -1) with the 0x18-byte record 0x10133728 + 0x18 * index. */
void __cdecl Sim_1002009d(int index);
/*
 * With p non-NULL: when the first dword of p is 0 HeapFree(game heap [0x1017b8f8], 1, dword at p+0x17),
 * otherwise 0x100b9203(p + 6, [0x1014cefc]).
 */
void __cdecl Sim_10027a8f(void *p);
/* Runs 0x10027fb8 on the node Sim_10027f33(key, name) finds, if any. */
void __cdecl Sim_100284df(int key, const void *name);
/* 0x10031ce0([obj+0x98], [obj+0x2c], x) & 1. */
uint32_t __cdecl Sim_1002d3a5(void *obj, uint32_t x);
/* 0x10031f95([obj+0x98], [obj+0x2c], a, b, c, d). */
void __cdecl Sim_1002d3ca(void *obj, uint32_t a, uint32_t b, uint32_t c, uint32_t d);

#endif
