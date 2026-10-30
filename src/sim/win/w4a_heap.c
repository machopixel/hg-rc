#include "w4a.h"
#include "../simmem.h"

typedef void *(__stdcall *HeapAllocFn)(void *heap, uint32_t flags, uint32_t bytes);
typedef int (__stdcall *HeapFreeFn)(void *heap, uint32_t flags, void *block);
typedef void *(__stdcall *LoadLibraryAFn)(const char *name);

#define GAME_HEAP (*SIM_PTR(void *, 0x1017b8f8))
#define IAT_HEAPFREE 0x10ac5634u
#define IAT_HEAPALLOC 0x10ac5658u
#define IAT_LOADLIBRARYA 0x10ac55e4u

// FUNCTION: HGSIM 0x100092bd
void __cdecl Heap_Free(void *block)
{
    if (block != 0)
        SIM_API(HeapFreeFn, IAT_HEAPFREE)(GAME_HEAP, 1, block);
}

// FUNCTION: HGSIM 0x10028559
void *__cdecl Heap_AllocZero(unsigned int size)
{
    return SIM_API(HeapAllocFn, IAT_HEAPALLOC)(GAME_HEAP, 9, size);
}

// FUNCTION: HGSIM 0x10027d5a
void __cdecl Sim_10027d5a(void)
{
    *SIM_PTR(void *, 0x1015dc30) = SIM_API(HeapAllocFn, IAT_HEAPALLOC)(GAME_HEAP, 9, 0xfc4);
}

// FUNCTION: HGSIM 0x1003c569
void *__cdecl Lib_Load_1003c569(const char *name)
{
    *SIM_PTR(void *, 0x1015eb68) = SIM_API(LoadLibraryAFn, IAT_LOADLIBRARYA)(name);
    return *SIM_PTR(void *, 0x1015eb68);
}
