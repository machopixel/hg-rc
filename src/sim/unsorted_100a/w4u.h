/* Round 4 (letter u), original 0x100a87f0. */
#ifndef HG_SIM_UNSORTED_100A_W4U_H
#define HG_SIM_UNSORTED_100A_W4U_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Frees the block whose pointer is the dword 0x1017d180 (Heap_Free_100285a3) and clears it; nothing when it is 0. */
void __cdecl Sim_100a87f0(void);

#endif
