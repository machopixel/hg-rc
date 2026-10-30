/*
 * Round 5 (letter y) functions of HGSIM.DLL in 0x1004xxxx. Plain C, original cdecl convention.
 * They call helpers that are not converted yet through their original address (SIM_FN), so the
 * original code still runs for those. Names carry the original address because the code alone
 * does not say what the data is for.
 */
#ifndef HG_SIM_UNSORTED_1004_W5Y_H
#define HG_SIM_UNSORTED_1004_W5Y_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Calls Sim_1007c42f(); when the flag at 0x1015f1fc is 0 sets it to 1 and clears the dwords 0x1015f1e4 and 0x1015f1d0. */
void __cdecl Sim_10041a16(void);
/* Clears the dwords 0x1015ed20 and 0x1015ed38; when the signed dword at 0x1013aec4 is above 0 sets 0x1015ee98 to 1 and calls Sim_1004409f(0). */
void __cdecl Sim_10044069(void);
/*
 * When the high byte of the 16-bit value is 7: moves the 0x3e bytes at 0x109b9481 down to 0x109b9480 (Sim_100fdf00),
 * stores the low byte of the value at 0x109b94be and returns 1. Otherwise returns 0.
 */
int __cdecl Sim_10048720(short value);
/* Sets the dword at 0x1013b9a8 to 3 and calls Sim_1004e838(). */
void __cdecl Sim_1004edb6(void);

#endif
