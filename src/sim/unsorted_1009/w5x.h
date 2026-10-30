/*
 * Round 5, letter x: the option setters at 0x10098811 and 0x100989a1, which call helpers that are not
 * converted yet (called through their address, SIM_FN).
 */
#ifndef HG_SIM_UNSORTED_1009_W5X_H
#define HG_SIM_UNSORTED_1009_W5X_H

#include <stdint.h>
#include "../simmem.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * Not converted. 0x10090d15 is patched by src/hooks/hooks_view.c (SetParticles), so it has to be called through
 * its address and never directly from C.
 */
#define SIM_CALL_10090d15 SIM_FN(int(__cdecl *)(int), 0x10090d15)
#define SIM_CALL_100a96d8 SIM_FN(void(__cdecl *)(int), 0x100a96d8)

/* Ignores its argument and runs 0x10090d15(-1). */
void __cdecl Sim_10098811(int unused);
/*
 * Sim_100332a3(0x800, value), 0x100a96d8(value), [0x1014c9fc] = value and the dword at +0x18 of the record
 * [0x1015d1b8] points at = value. The first argument is not used.
 * NOTE: src/hooks/hooks_view.c already hooks this address (HookedSetTerrainTextures), so it has no entry in conv_w5x.inc.
 */
void __cdecl Sim_100989a1(int unused, int value);

#endif
