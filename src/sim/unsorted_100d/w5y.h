/*
 * Round 5 (letter y) functions of HGSIM.DLL in 0x100dxxxx. Plain C, original cdecl convention.
 * They call helpers that are not converted yet through their original address (SIM_FN), so the
 * original code still runs for those. Names carry the original address because the code alone
 * does not say what the data is for.
 */
#ifndef HG_SIM_UNSORTED_100D_W5Y_H
#define HG_SIM_UNSORTED_100D_W5Y_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * For i = 0..15 calls Sim_1005259e(i, base + 8 + i * 0x11) (17-byte texts) and clears the dword at 0x109b7d50 + i * 0x6c.
 */
void __cdecl Sim_100db205(void *base);

#endif
