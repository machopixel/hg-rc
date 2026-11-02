#include "w4v.h"
#include "../util/access.h"
#include "../win/w4v.h"

// FUNCTION: HGSIM 0x10053132
void __cdecl Sim_10053132(void)
{
    uint32_t block = G32(0x1015f3b0);
    uint32_t next;

    while (block != 0) {
        next = U32(block, 0);
        Heap_FreeRaw((void *)(uintptr_t)block);
        block = next;
    }
    G32(0x1015f3b0) = 0;
    G32(0x109b6880) = 0;
}
