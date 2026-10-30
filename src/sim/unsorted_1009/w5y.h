/*
 * Round 5 (letter y) functions of HGSIM.DLL in 0x1009xxxx. Plain C, original cdecl convention.
 * They call helpers that are not converted yet through their original address (SIM_FN), so the
 * original code still runs for those. Names carry the original address because the code alone
 * does not say what the data is for.
 */
#ifndef HG_SIM_UNSORTED_1009_W5Y_H
#define HG_SIM_UNSORTED_1009_W5Y_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Calls Sim_10090d15(value), then stores value in the dword at offset 0x28 of the record pointed to by 0x1015d1b8. */
void __cdecl Sim_10098826(uint32_t unused, uint32_t value);
/*
 * Saves the dword 0x1015d1c8 in 0x1015d104 and sets 0x1015d1c8 to 4; then, by the signed dword at 0x1015d1ac:
 * up to 2 calls Sim_10099c36(); exactly 4 with the signed dword 0x1015d1cc up to 2 calls Sim_100999fc().
 */
void __cdecl Sim_100999bc(void);

#endif
