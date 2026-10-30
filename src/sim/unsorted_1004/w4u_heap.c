#include "w4u.h"
#include "../win/w4u_api.h"

// FUNCTION: HGSIM 0x100461d0
void __cdecl Heap_Free_100461d0(void *block)
{
    W4u_HeapFree((uint32_t)(uintptr_t)block);
}
