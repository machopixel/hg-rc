/*
 * Round 9, letter g: functions of HGSIM.DLL at 0x100622f0, 0x10062956 and 0x1006ca00 (cockpit display
 * drivers of w8w_hud.c and a view toggle). Callees that are not converted are called through their original
 * address (SIM_FN).
 */
#ifndef HG_SIM_UNSORTED_1006_W9G_H
#define HG_SIM_UNSORTED_1006_W9G_H

#include <stdint.h>
#include "../simmem.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Not converted: 0x10064364 (value, flag), 0x10062b7c (object, a, b, c), 0x10063529 (object). */
#define W9G6_CALL_10064364 SIM_FN(void(__cdecl *)(uint32_t, uint32_t), 0x10064364)
#define W9G6_CALL_10062b7c SIM_FN(void(__cdecl *)(void *, uint32_t, uint32_t, uint32_t), 0x10062b7c)
#define W9G6_CALL_10063529 SIM_FN(void(__cdecl *)(void *), 0x10063529)

/* ---- w9g_misc.c ---- */
/*
 * Draws the cockpit displays of `obj` when [0x10144fa0] is set: Sim_10062956 when obj's dword +0xc4d is set,
 * Sim_10065131 with [0x10176938], 0x10064364(a1, a3) with [0x10144fac], 0x10062b7c(obj, a4, a5, a6) and (with
 * [0x10176950]) Sim_10062abd(obj) with [0x10144fa4], 0x10063529(obj) with [0x10144fa8]. a2 is not used.
 */
void __cdecl Sim_100622f0(void *obj, uint32_t a1, uint32_t a2, uint32_t a3, uint32_t a4, uint32_t a5,
                          uint32_t a6);
/*
 * Damage flicker of the current entity's display: by the dword +0xc4d of its block (+0x28) 1 / 2 / 3 sets
 * [0x10165be0], [0x10165be4] to 10, 2 / 50, 4 / 100, 10 and, when Sim_1009c929(100) is below the first,
 * marks [0x10165be4] slots 0x10165bfc + 0x18 * Sim_1009c929(10) with 1; then Sim_10062780. 0 and above 3:
 * nothing.
 */
void __cdecl Sim_10062956(void);
/* Toggles [0x10179ca8]: Sim_100aa351 (when it was 0) or Sim_100aa39f on the current entity's node +0x48 first.
 * Returns the new value. */
int __cdecl Sim_1006ca00(void);

#endif
