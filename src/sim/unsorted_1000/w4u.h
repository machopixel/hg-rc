/* Round 4 (letter u), original 0x1000e62e: releases the table object at 0x1015d0a4. */
#ifndef HG_SIM_UNSORTED_1000_W4U_H
#define HG_SIM_UNSORTED_1000_W4U_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * Frees the table object whose pointer is the dword 0x1015d0a4 (nothing when it is 0): for each of its
 * dword[1] entries the flag dword at +0x54+4*i is cleared and the block pointer at +0x94+8*i is freed
 * (the pointer is read at a stride of 8 bytes, the flag at a stride of 4: kept as the original has it),
 * then the object is freed and 0x1015d0a4 is cleared.
 */
void __cdecl Sim_1000e62e(void);

#endif
