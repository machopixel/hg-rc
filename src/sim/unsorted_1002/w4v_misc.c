#include "w4v.h"
#include "../util/access.h"
#include "../win/w4v.h"

// FUNCTION: HGSIM 0x1002a29f
int __cdecl Sim_1002a29f(int index)
{
    uint32_t size = 0x2a0;
    void *block;

    if ((uint32_t)index != G32(0x1017b8c8))
        size += 0x5c;
    block = Heap_AllocRaw(size);
    if (block == 0)
        return 0;
    PTRTAB(index) = (uint32_t)(uintptr_t)block;
    return 1;
}
