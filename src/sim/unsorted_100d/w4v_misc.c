#include "w4v.h"
#include "../util/access.h"
#include "../win/w4v.h"
#include "../unsorted_100f/w2e.h"
#include "../unsorted_100f/w2j.h"

#define PTR(v) ((void *)(uintptr_t)(v))

// FUNCTION: HGSIM 0x100da45c
void __cdecl Sim_100da45c(void)
{
    G32(0x1017e6bc) = (uint32_t)(uintptr_t)Heap_AllocRaw(0x60);
    G32(0x1017e6c0) = (uint32_t)(uintptr_t)Heap_AllocRaw(0x48);
    Mem_Fill(PTR(G32(0x1017e6bc)), 0, 0x60);
    Mem_Fill(PTR(G32(0x1017e6c0)), 0, 0x48);
    U32(G32(0x1017e6c0), 0x44) = G32(0x1017e6bc);
    U32(G32(0x1017e6bc), 0x40) = 0;
    Str_Cat((char *)PTR(G32(0x1017e6c0) + 0x20), (const char *)GBYTES(0x10152488));
    Str_Cat((char *)PTR(G32(0x1017e6bc)), (const char *)GBYTES(0x10152498));
}

// FUNCTION: HGSIM 0x100db351
void __cdecl Sim_100db351(void)
{
    int i;

    if (G32(0x1017e688) != 0)
        Heap_FreeRaw(PTR(G32(0x1017e688)));
    G32(0x1017e688) = 0;
    for (i = 0; i < 0x10; i++) {
        if (G32(0x1017ee40 + 4 * i) != 0)
            Heap_FreeRaw(PTR(G32(0x1017ee40 + 4 * i)));
        G32(0x1017ee40 + 4 * i) = 0;
    }
    if (G32(0x1017e68c) != 0)
        Heap_FreeRaw(PTR(G32(0x1017e68c)));
    G32(0x1017e68c) = 0;
}
