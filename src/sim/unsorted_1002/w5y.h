/*
 * Round 5 (letter y) functions of HGSIM.DLL in 0x1002xxxx. Plain C, original cdecl convention.
 * They call helpers that are not converted yet through their original address (SIM_FN), so the
 * original code still runs for those. Names carry the original address because the code alone
 * does not say what the data is for.
 */
#ifndef HG_SIM_UNSORTED_1002_W5Y_H
#define HG_SIM_UNSORTED_1002_W5Y_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * Walks the chain that starts at the dword at obj+0x67 of the object pointed to by 0x1015d528 (an unaligned
 * pointer field) and releases each node with Sim_1001ffc8, which returns the next node.
 */
void __cdecl Sim_1002001f(void);
/* Returns Sim_1001f790(a, 0, -1). */
int __cdecl Sim_10020441(void *a);
/*
 * Sim_100246b3(-2, 0, a1, a5, a6, a7, a5, a6, a7, a2, a3, a4). The words a5..a7 are floats that are passed
 * through bit for bit.
 */
void __cdecl Sim_1002464b(uint32_t a1, uint32_t a2, uint32_t a3, uint32_t a4, uint32_t a5, uint32_t a6, uint32_t a7);
/* Stores a2 in the dword at 0x1015dc08 and calls Sim_100246b3(-2, 0, a1, 0, ...) with zeros for the other arguments. */
void __cdecl Sim_10024684(uint32_t a1, uint32_t a2);
/* Returns Sim_10046f7f("HGCAR.CFG", 0x101653e0), both pointers into the DLL's data. */
int __cdecl Sim_10025dac(void);
/*
 * Subtracts d from the float at 0x109f09ec + i * 0x14 (computed in double, stored as float); when the result
 * has its sign bit set calls Sim_10029125(i).
 */
void __cdecl Sim_100291a2(int i, float d);
/* Returns Sim_10030c87(obj[0x98], obj[0x2c], arg) & 1 (dwords at those offsets). */
uint32_t __cdecl Sim_1002d454(void *obj, uint32_t arg);

#endif
