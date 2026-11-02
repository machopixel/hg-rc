#include "w8x.h"
#include "../unsorted_100a/w5z.h"
#include "../unsorted_1010/w2f.h"
#include "../unsorted_1010/w2h.h"
#include "../util/access.h"
#include "../util/util.h"
#include "../util/w3o.h"
#include "../crt/hg_winheap.h"


// FUNCTION: HGSIM 0x1009c860
void __cdecl Sim_1009c860(uint32_t seed)
{
    double root, scale;
    int32_t i, j;

    Sim_10100390(seed);
    for (i = 0; i < 0x7f; i++)
        SIM_PTR(u32a, 0x101cffc0)[i] = (uint32_t)Crt_Rand();
    root = W8X9_CALL_100fca04(90.0);
    scale = GF64(0x1012b9e8) * root / GF64(0x1012b9f0);
    for (i = 0; i < 0x7f; i++) {
        int32_t sum = 0;

        for (j = 0; j < 0x1e; j++)
            sum = (int32_t)((uint32_t)sum + (uint32_t)Crt_Rand());
        SIM_PTR(u32a, 0x101cfce0)[i] = (uint32_t)Render_Ftol(((double)sum * scale - root) * GF64(0x1012b9f8));
    }
}

// FUNCTION: HGSIM 0x1009db70
void *__cdecl Sim_1009db70(void *node)
{
    uint8_t *copy;

    if (node == 0)
        return 0;
    copy = Hg_HeapAlloc(G32(0x1017b8f8), 1, 0x28);
    if (copy == 0)
        return 0;
    U32(copy, 8) = (uint32_t)(uintptr_t)node;
    U32(copy, 0xc) = U32(node, 0);
    U32(copy, 0) = (uint32_t)(uintptr_t)Sim_1009db70((void *)(uintptr_t)U32(node, 4));
    U32(copy, 4) = (uint32_t)(uintptr_t)Sim_1009db70((void *)(uintptr_t)U32(node, 8));
    Sim_100a9d21(node, (uint32_t *)(copy + 0x10), (uint32_t *)(copy + 0x14), (uint32_t *)(copy + 0x18));
    Sim_100a9ccf(node, (uint32_t)(uintptr_t)(copy + 0x1c), (uint32_t)(uintptr_t)(copy + 0x20),
                 (uint32_t)(uintptr_t)(copy + 0x24));
    return copy;
}
