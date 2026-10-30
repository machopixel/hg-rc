/* Round 4 (letter u), original 0x100461d0: HeapFree wrapper. */
#ifndef HG_SIM_UNSORTED_1004_W4U_H
#define HG_SIM_UNSORTED_1004_W4U_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* HeapFree(game heap, 1, block): no check for NULL; the same code as Heap_Free_100285a3. */
void __cdecl Heap_Free_100461d0(void *block);

#endif
