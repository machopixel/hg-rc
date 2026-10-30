#include "w4v.h"
#include "w4a.h"
#include "w4v_api.h"

// FUNCTION: HGSIM 0x100b659a
void *__cdecl Heap_AllocRaw(uint32_t size)
{
    return SIM_API(W4vHeapAllocFn, W4V_IAT_HEAPALLOC)(W4V_GAME_HEAP, 1, size);
}

// FUNCTION: HGSIM 0x100a7800
int __cdecl Heap_FreeRaw(void *block)
{
    return SIM_API(W4vHeapFreeFn, W4V_IAT_HEAPFREE)(W4V_GAME_HEAP, 1, block);
}

// FUNCTION: HGSIM 0x100a77ef
void *__cdecl Sim_100a77ef(uint32_t size)
{
    return Heap_AllocZero(size);
}
