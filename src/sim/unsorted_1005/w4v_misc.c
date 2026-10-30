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

// FUNCTION: HGSIM 0x10056d04
uint32_t __cdecl Sim_10056d04(void *obj)
{
    if (obj != 0) {
        if (U32(obj, 0x60) != 0)
            Heap_FreeRaw((void *)(uintptr_t)U32(obj, 0x60));
        if (U32(obj, 0x64) != 0)
            Heap_FreeRaw((void *)(uintptr_t)U32(obj, 0x64));
        if (U32(obj, 0x68) != 0) {
            if (U32(U32(obj, 0x68), 0) != 0)
                Heap_FreeRaw((void *)(uintptr_t)U32(U32(obj, 0x68), 0));
            Heap_FreeRaw((void *)(uintptr_t)U32(obj, 0x68));
        }
        if (U32(obj, 0x6c) != 0) {
            if (U32(U32(obj, 0x6c), 0) != 0)
                Heap_FreeRaw((void *)(uintptr_t)U32(U32(obj, 0x6c), 0));
            Heap_FreeRaw((void *)(uintptr_t)U32(obj, 0x6c));
        }
        if (U32(obj, 0x70) != 0)
            Heap_FreeRaw((void *)(uintptr_t)U32(obj, 0x70));
    }
    return 0;
}
