#include "w4w.h"
#include "../win/w4w_api.h"
#include "../util/access.h"
#include "../unsorted_1000/w2e.h"
#include "../unsorted_1008/w2i.h"
#include "../unsorted_100f/w2e.h"
#include "../crt/hg_winheap.h"

// FUNCTION: HGSIM 0x10074b80
int __cdecl Sim_10074b80(uint32_t value)
{
    uint8_t *entry = (uint8_t *)Hg_HeapAlloc(G32(0x1017b8f8), 1, 0x18);

    if (entry == 0)
        return 0;
    Mem_Fill(entry, 0, 0x18);
    U32(entry, 0) = value;
    if (G32(0x1015dc14) == 0)
        G32(0x1015dc14) = (uint32_t)(uintptr_t)entry;
    else
        U32(G32(0x1015dc10), 0x14) = (uint32_t)(uintptr_t)entry;
    G32(0x1015dc10) = (uint32_t)(uintptr_t)entry;
    return 1;
}
