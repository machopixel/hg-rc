/* Round 4 (letter u), originals in 0x10027f06..0x1002f9e0: small heap users. */
#ifndef HG_SIM_UNSORTED_1002_W4U_H
#define HG_SIM_UNSORTED_1002_W4U_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Clears the dwords 0x109f13e0, 0x109f13f0 and 0x109f13f4, then calls Sim_10027d5a (win/w4a.h). */
void __cdecl Sim_10027f06(void);
/* HeapFree(game heap, 1, block): no check for NULL (Heap_Free in win/w4a.h has one). */
void __cdecl Heap_Free_100285a3(void *block);
/* Allocates 0x18 bytes (not zeroed) and sets its six dwords to 0. Returns the block, or NULL. */
void *__cdecl Sim_1002f9e0(void);

#endif
