/*
 * Wave 2 functions of HGSIM.DLL around 0x100a7000..0x100aa000 (letter h).
 * Names carry the original address because the code alone does not say what
 * the fields mean.
 */
#ifndef HG_SIM_UNSORTED_100a_W2H_H
#define HG_SIM_UNSORTED_100a_W2H_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Returns the value of Sim_10015bc9(). */
int __cdecl Sim_100a774e(void);
/*
 * Lowers the dword 0x1017d174 by 0x40, clears the byte at +0x29 of the address it now holds and,
 * unless that address minus 0xc8 is above (unsigned) the dword 0x1017d178, clears the dword
 * 0x101c9ea4. Returns the new address.
 */
uint32_t __cdecl Sim_100a88ff(void);
/*
 * Copies the four dwords at src to obj+0x7c, clears bit 15 of the dword obj+0x94
 * and sets its bit 0.
 */
void __cdecl Sim_100aa270(void *obj, const uint32_t *src);

#endif
