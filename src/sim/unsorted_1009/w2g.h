/*
 * Wave 2g: functions of HGSIM.DLL from 0x100903e0..0x1009e5ac: an object
 * pool (0x57e-byte objects from 0x10217c20, free stack of object numbers at
 * 0x10216b80 with its top index in [0x10215ae8]), two constructors on it,
 * and small setters that call converted setters of other areas. Names carry
 * the original address where the code alone does not say what the objects are.
 */
#ifndef HG_SIM_W2G_1009_H
#define HG_SIM_W2G_1009_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Appends the object to the end of the doubly linked list (first [0x1017cc28], last [0x1017cc2c]; previous at +0, next at +4). Does nothing for null. */
void __cdecl Sim_100903e0(uint8_t *obj);
/*
 * When the stack top [0x10215ae8] is >= 0 and ([0x1014b9b8] is set or `force`
 * is non-zero): appends the object whose number is on top (Sim_100903e0),
 * pops it and returns it. Otherwise returns null.
 */
uint8_t *__cdecl Sim_10090581(int force);
/* o = Sim_10090581(force); when o is not null: o[0x556] = a, o[0x55a] = b, o[0x57a] = c. Returns o. */
uint8_t *__cdecl Sim_10090c7d(uint32_t a, uint32_t b, uint32_t c, int force);
/* 1 when the argument is non-zero, else 0. */
int __cdecl Sim_10091071(int value);
/*
 * Unless [0x1017cc24] + 1 >= [0x1017cc20]: o = Sim_10090c7d(0x10001433,
 * 0x10002e78, `arg`, 1) (two code addresses of the DLL) and, when o is not
 * null, sets o's word +0x10 = 1, +0x562 = param, +0x566 = 0, resets the
 * block o[0x552][0x8c] (floats +0x44, +0x48, +0x4c, +0x50 = 1.0, +0x38,
 * +0x3c, +0x40 = 0), counts up [0x1017cc24], o[0x55e] = [0x1015dc78],
 * the word at o + 0x2a = 1 and o[0xc] = 0xd. The second argument is not used.
 */
void __cdecl Sim_10091085(uint32_t arg, int unused, uint32_t param);
/* Same with the code addresses 0x10003049 and 0x10002077, o[0x566] = param2. */
void __cdecl Sim_1009201f(uint32_t arg, int unused, uint32_t param, uint32_t param2);
/* [[0x1015d1b8] + 0x64] = value. The first argument is not used. */
void __cdecl Sim_100987fa(int unused, uint32_t value);
/* Sim_100ac603(-1). The argument is not used. */
int __cdecl Sim_10098881(int unused);
/* Sim_100ac603(value); then [[0x1015d1b8] + 0x30] = value. The first argument is not used. */
void __cdecl Sim_10098896(int unused, int value);
/* Sim_100d2c86(a, b); Sim_100332e9(a, b); [[0x1015d1b8] + 0x1c] = b. */
void __cdecl Sim_10098a18(int a, int b);
/* Returns -1. */
int __cdecl Sim_10098e62(void);
/* Returns 1. */
int __cdecl Sim_10098e74(void);
/* When [0x1015d1ac] <= 2 and [0x1015d1cc] <= 2: [0x1015d1cc] = ([0x1015d1ac] + 1) % 3. */
void __cdecl Sim_1009b2b4(void);
/* Fills three blocks (at 0x1014c31c, 0x1014c3c4 and 0x1014c46c, 0xa8 bytes apart): dwords 0, 2, 4, 7, 8 of each from the dwords at 0x101c528c, 0x101c4bcc, 0x101c4f78, 0x101c45f4, 0x101c4b90, and dwords 1, 3, 5 with the addresses 0x101d02c0, 0x101d02a0, 0x101d0340. */
void __cdecl Sim_1009c710(void);

#endif
