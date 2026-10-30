/*
 * Wave 2 (letter i) additions to unsorted_1009: two small state functions around 0x1009b000..0x1009d000.
 * Plain C, original cdecl convention. Names carry the original address
 * because the code alone does not say what the data is for.
 */
#ifndef HG_SIM_UNSORTED_1009_W2I_H
#define HG_SIM_UNSORTED_1009_W2I_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * With g = the signed dword 0x1015d1ac: nothing when g == v. When both are <= 2
 * the dword 0x1015d1cc becomes v. When g > 2 it becomes 5 and the dword
 * 0x1015d098 becomes v. Otherwise (g <= 2 < v) it becomes 3.
 */
void __cdecl Sim_1009b30f(int v);
/*
 * The table of 127 dwords at 0x101cfce0 with the index in 0x1017cd44: returns
 * the entry at the index, then advances the index modulo 127 (signed).
 */
uint32_t __cdecl Sim_1009c9e5(void);

#endif
