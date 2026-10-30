/*
 * Wave 2, letter j: leaf functions of HGSIM.DLL around 0x10090000..0x100a0000.
 */
#ifndef HG_SIM_UNSORTED_1009_W2J_H
#define HG_SIM_UNSORTED_1009_W2J_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Fills the small constant tables at 0x10215af0..0x10217bc0, 0x101d09c0 and 0x1036f050 (loops of simple formulas, see the body). */
void __cdecl Sim_100905e8(void);
/* Dword +0x20 of the block that dword 0x1015d1b8 points to; the argument is not used. */
uint32_t __cdecl Sim_10098a49(uint32_t unused);
/*
 * 1 when bit 5 of the signed word at obj+0x24 is set and ((1 << (bit & 31)) |
 * the sign-extended word at obj+0x26) is not 0; otherwise 0.
 */
int __cdecl Sim_1009a6b3(const void *obj, int bit);
/* Calls Sim_1009b362 when dword 0x1015d1ac is 4. */
void __cdecl Sim_1009b2fc(void);
/*
 * With a = dword 0x1015d1ac and b = dword 0x1015d1cc: if a == 4 and b == 4
 * then b = 5, else if a <= 2 and b <= 2 (signed) then b = 3.
 */
void __cdecl Sim_1009b362(void);
/* Returns dword 0x1017d064. */
uint32_t __cdecl Sim_1009e5a2(void);

#endif
