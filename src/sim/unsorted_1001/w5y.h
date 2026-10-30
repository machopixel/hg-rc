/*
 * Round 5 (letter y) functions of HGSIM.DLL in 0x1001xxxx. Plain C, original cdecl convention.
 * They call helpers that are not converted yet through their original address (SIM_FN), so the
 * original code still runs for those. Names carry the original address because the code alone
 * does not say what the data is for.
 */
#ifndef HG_SIM_UNSORTED_1001_W5Y_H
#define HG_SIM_UNSORTED_1001_W5Y_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* mode = Sim_100109e7(dword at 0x1015d100); when mode is not negative calls Sim_10010564(mode, 0). */
void __cdecl Sim_10010a73(void);
/* Same with Sim_10010632 as the second call. */
void __cdecl Sim_10010b38(void);
/* Stores (short)value at obj+0xf0 and returns whether Sim_100cb4fb(obj) is greater than 0. */
int __cdecl Sim_1001403a(void *obj, short value);
/*
 * x = f / 3840.0f (as float); returns Sim_1001655f(x, n * 2, 1), a float that the callee leaves in
 * st(0) and this function passes on in st(0). The first argument is not used.
 */
float __cdecl Sim_100144bb(uint32_t unused, float f, int n);
/* Calls Sim_10039aca(0xb, a) and Sim_1009e1ef(), clears two tables (Sim_1000df50, Sim_1001f6c0), sets the dword at 0x109f4228 to the dword at 0x1015dc78 plus 0x389. Returns 1. */
int __cdecl Sim_1001f415(int a);
/* Sim_1000ede4(), Sim_1002001f(), Sim_1001f335(). */
void __cdecl Sim_1001f4ba(void);

#endif
