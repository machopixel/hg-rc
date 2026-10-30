/*
 * Round 5 (letter y) functions of HGSIM.DLL in 0x1000xxxx. Plain C, original cdecl convention.
 * They call helpers that are not converted yet through their original address (SIM_FN), so the
 * original code still runs for those. Names carry the original address because the code alone
 * does not say what the data is for.
 */
#ifndef HG_SIM_UNSORTED_1000_W5Y_H
#define HG_SIM_UNSORTED_1000_W5Y_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Formats "Items in cache: %li" with the dword at 0x109f13e0 and, when the text screen is enabled
   (dword at 0x10a3ade8), puts it at column 0x23, row 0x18; sets the flag at 0x1015d058 to 1. */
void __cdecl Cache_ShowItemCount(void);
/* Blanks the same text-screen field with 27 spaces and sets the flag at 0x1015d058 to 0. */
void __cdecl Cache_ClearItemCount(void);
/* Calls Sim_100089c6(obj) when the dword at obj+0x30 is not 0. */
void __cdecl Sim_100094ba(void *obj);
/*
 * Looks up the name "AFXHDMLG" with Sim_1000df9c and calls Sim_1000e737 with that name number,
 * Sim_1000edce(), the dword at 0x101321b4 and constants (-1, 0x250, zeros). Returns its result.
 */
int __cdecl Sim_1000e93e(void);

#endif
