/*
 * Round 4, letter v: the Windows imports used by the converted code of this letter, as __stdcall function
 * pointer types, and the import slot addresses of HGSIM.DLL (at its preferred base). Internal to the w4v_*.c files.
 */
#ifndef HG_SIM_WIN_W4V_API_H
#define HG_SIM_WIN_W4V_API_H

#include <stdint.h>
#include "../simmem.h"

typedef void *(__stdcall *W4vHeapAllocFn)(void *heap, uint32_t flags, uint32_t bytes);
typedef int (__stdcall *W4vHeapFreeFn)(void *heap, uint32_t flags, void *block);

#define W4V_IAT_HEAPFREE 0x10ac5634u
#define W4V_IAT_HEAPALLOC 0x10ac5658u
/* The handle of the game's heap (the value HeapAlloc and HeapFree are called with). */
#define W4V_GAME_HEAP (*SIM_PTR(void *, 0x1017b8f8))

#endif
