/*
 * Round 5 (letter y) functions of HGSIM.DLL in 0x1005xxxx. Plain C, original cdecl convention.
 * They call helpers that are not converted yet through their original address (SIM_FN), so the
 * original code still runs for those. Names carry the original address because the code alone
 * does not say what the data is for.
 */
#ifndef HG_SIM_UNSORTED_1005_W5Y_H
#define HG_SIM_UNSORTED_1005_W5Y_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * Searches the 0x6d records of 0x20 bytes at 0x1013f160 for one whose first field (a text pointer) equals name
 * (Sim_100ff1e0 compare, 0 = equal). Returns the record or NULL.
 */
void *__cdecl Sim_10053620(const char *name);
/* 0x10057afc (keyboard flush): round 28, src/sim/platform/w28.h. */
/* Sim_1010d180(b) first (it changes the text in place), then returns Sim_1005af06(a, b). */
int __cdecl Sim_1005aee5(uint32_t a, char *b);

#endif
