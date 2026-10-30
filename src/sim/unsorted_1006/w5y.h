/*
 * Round 5 (letter y) functions of HGSIM.DLL in 0x1006xxxx. Plain C, original cdecl convention.
 * They call helpers that are not converted yet through their original address (SIM_FN), so the
 * original code still runs for those. Names carry the original address because the code alone
 * does not say what the data is for.
 */
#ifndef HG_SIM_UNSORTED_1006_W5Y_H
#define HG_SIM_UNSORTED_1006_W5Y_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * Passes its seven arguments, in the same order, to Sim_10060220 with 0 as an eighth. The arguments after the first
 * are floats and are passed through bit for bit.
 */
void __cdecl Sim_10060656(uint32_t a1, uint32_t a2, uint32_t a3, uint32_t a4, uint32_t a5, uint32_t a6, uint32_t a7);
/*
 * v = (int)((n / 15.0f) * 362.0f) for n up to 0xf (the product is computed in double and truncated like the x87
 * original), 0x16a above that; calls Sim_100d3510(0x11, v, 1) and returns its result.
 */
int __cdecl Sim_10061b78(uint32_t n);

#endif
