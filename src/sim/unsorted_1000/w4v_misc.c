#include "w4v.h"
#include "../util/access.h"
#include "../win/w4a.h"
#include "../win/w4v.h"
#include "../win/w4v_api.h"
#include "../unsorted_100f/w2e.h"

#define PTR(v) ((void *)(uintptr_t)(v))

// FUNCTION: HGSIM 0x100092dd
void __cdecl Sim_100092dd(void *obj)
{
    uint32_t node = U32(obj, 0x30);
    uint32_t prev;

    if (node == 0 || U32(obj, 0x2c) == 0)
        return;
    if (node == U32(obj, 0x2c)) {
        U32(obj, 0x2c) = U32(node, 0xc);
    } else {
        for (prev = U32(obj, 0x2c); U32(prev, 0xc) != 0 && U32(prev, 0xc) != node; prev = U32(prev, 0xc))
            ;
        if (U32(prev, 0xc) == 0)
            return;
        U32(prev, 0xc) = U32(node, 0xc);
    }
    Heap_Free(PTR(node));
}

// FUNCTION: HGSIM 0x1000e520
int __cdecl Sim_1000e520(uint32_t count)
{
    if (G32(0x1015d0a4) != 0)
        return -1;
    G32(0x1015d0a4) = (uint32_t)(uintptr_t)SIM_API(W4vHeapAllocFn, W4V_IAT_HEAPALLOC)(W4V_GAME_HEAP, 1, 0x374);
    if (G32(0x1015d0a4) == 0)
        return -3;
    Mem_Fill(PTR(G32(0x1015d0a4)), 0, 0x374);
    U32(G32(0x1015d0a4), 4) = count < 0xc ? count : 0xb;
    if (U32(G32(0x1015d0a4), 4) != 0)
        U32(G32(0x1015d0a4), 4) = 0;
    return 1;
}
