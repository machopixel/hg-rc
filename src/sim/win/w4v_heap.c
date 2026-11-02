#include "w4v.h"
#include "w4a.h"
#include "w4v_api.h"
#include "../crt/hg_winheap.h"
#include "../util/access.h"

// FUNCTION: HGSIM 0x100b659a
void *__cdecl Heap_AllocRaw(uint32_t size)
{
    return Hg_HeapAlloc(G32(0x1017b8f8), 1, size);
}

// FUNCTION: HGSIM 0x100a7800
int __cdecl Heap_FreeRaw(void *block)
{
    return Hg_HeapFree(G32(0x1017b8f8), 1, block);
}

// FUNCTION: HGSIM 0x100a77ef
void *__cdecl Sim_100a77ef(uint32_t size)
{
    return Heap_AllocZero(size);
}
