/*
 * Round 5, letter b: small functions in 0x10027df4..0x1002d349 that call helpers which are not converted
 * yet (called through their address with SIM_FN).
 */
#ifndef HG_SIM_UNSORTED_1002_W5B_H
#define HG_SIM_UNSORTED_1002_W5B_H

#include <stdint.h>
#include "../simmem.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Not converted. */
#define SIM_CALL_10027fb8 SIM_FN(void(__cdecl *)(void *), 0x10027fb8)
#define SIM_CALL_10028d9f SIM_FN(void(__cdecl *)(void *, int, int), 0x10028d9f)
#define SIM_CALL_100a77a7 SIM_FN(int(__cdecl *)(const void *, uint32_t), 0x100a77a7)
#define SIM_CALL_10031acd SIM_FN(int(__cdecl *)(void *, int, float, float, float), 0x10031acd)

/*
 * The hash table of 0x3f1 chains at [0x1015dc30] (nothing when that is NULL): for every node (next pointer at +8,
 * read before the call) whose word +0 is 0xf000 or more runs 0x10027fb8(node).
 */
void __cdecl Sim_10027df4(void);
/* For a non-NULL obj: runs itself on Sim_100aa7f1(obj) and on Sim_100aa7fc(obj) when they are not 0, then 0x10028d9f(obj, a, b). */
void __cdecl Sim_10028f38(void *obj, int a, int b);
/* 0x100a77a7(name, [0x1014ceb4]) for the two names at 0x10138254 and 0x1013826c, then for the 0x2e names of 0xd6 bytes from 0x101455ba. */
void __cdecl Sim_1002b600(void);
/* When obj[0x98] is not 0: 0x10031acd(obj[0x98], obj[0x2c], a, b, c) & 1, else Sim_1002d276(obj, a, b, c). */
int __cdecl Sim_1002d349(void *obj, float a, float b, float c);

#endif
