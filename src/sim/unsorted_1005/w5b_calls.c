#include "w5b.h"
#include "../win/w4a.h"
#include "../unsorted_100f/w2e.h"
#include "../util/access.h"

// FUNCTION: HGSIM 0x1005259e
int __cdecl Sim_1005259e(int index, const char *name)
{
    int i;

    if (index >= 0x10)
        return 0;
    for (i = 0; i < GS32(0x1015f3ac); i++) {
        if (SIM_STRCMPI(SIM_PTR(const char, 0x109b68e0u + (uint32_t)i * 0x74u), name) == 0) {
            u32a *from = SIM_PTR(u32a, 0x109b68e0u + (uint32_t)i * 0x74u);
            u32a *to = SIM_PTR(u32a, 0x109b8420u + (uint32_t)index * 0x74u);
            int n;

            G32(0x109b7d58u + (uint32_t)index * 0x6cu) = (uint32_t)i;
            for (n = 0; n < 0x1d; n++)
                to[n] = from[n];
            return 1;
        }
    }
    return 0;
}

// FUNCTION: HGSIM 0x10053010
int __cdecl Sim_10053010(const uint16_t *src)
{
    uint8_t *node = (uint8_t *)Heap_AllocZero(0xf);

    if (node == 0) {
        SIM_CALL_100bc280(0x42, 0);
        return 0;
    }
    U32(node, 0) = 0;
    U16(node, 4) = src[0];
    Str_CopyN((char *)node + 6, (const char *)(src + 1), 8);
    U8(node, 0xe) = 0;
    if (G32(0x1015f3b0) == 0)
        G32(0x1015f3b0) = (uint32_t)(uintptr_t)node;
    else
        U32(G32(0x109b6880), 0) = (uint32_t)(uintptr_t)node;
    G32(0x109b6880) = (uint32_t)(uintptr_t)node;
    return 1;
}

// FUNCTION: HGSIM 0x1005f9aa
int __cdecl Sim_1005f9aa(const char *text)
{
    if (G32(0x101668f4) == 0 || G32(0x101668f8) == 0)
        return 0;
    SIM_FPRINTF((void *)(uintptr_t)G32(0x101668f8), SIM_PTR(const char, 0x10144340), text);
    return 1;
}
