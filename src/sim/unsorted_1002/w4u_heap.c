#include "w4u.h"
#include "../win/w4a.h"
#include "../win/w4u_api.h"

// FUNCTION: HGSIM 0x10027f06
void __cdecl Sim_10027f06(void)
{
    G32(0x109f13e0) = 0;
    G32(0x109f13f0) = 0;
    G32(0x109f13f4) = 0;
    Sim_10027d5a();
}

// FUNCTION: HGSIM 0x100285a3
void __cdecl Heap_Free_100285a3(void *block)
{
    W4u_HeapFree((uint32_t)(uintptr_t)block);
}

// FUNCTION: HGSIM 0x1002f9e0
void *__cdecl Sim_1002f9e0(void)
{
    uint32_t block = W4u_HeapAlloc(0x18);

    if (block != 0) {
        U32(block, 0x10) = 0;
        U32(block, 8) = 0;
        U32(block, 0) = 0;
        U32(block, 0x14) = 0;
        U32(block, 0xc) = 0;
        U32(block, 4) = 0;
    }
    return (void *)(uintptr_t)block;
}
