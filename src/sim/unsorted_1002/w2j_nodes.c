#include "w2j.h"
#include "../util/access.h"
#include "../unsorted_100d/w2j.h"

// FUNCTION: HGSIM 0x10027c70
void __cdecl Sim_10027c70(void *node)
{
    if (S16(node, 2) == 0)
        return;
    S16(node, 2) = 0;
    if (G32(0x109f13f4) != 0)
        U32(G32(0x109f13f4), 0xc) = (uint32_t)(uintptr_t)node;
    U32(node, 0x10) = G32(0x109f13f4);
    G32(0x109f13f4) = (uint32_t)(uintptr_t)node;
    U32(node, 0xc) = 0;
    if (G32(0x109f13f0) == 0)
        G32(0x109f13f0) = (uint32_t)(uintptr_t)node;
}

// FUNCTION: HGSIM 0x10027d77
void __cdecl Sim_10027d77(void)
{
    int bucket;

    G32(0x109f13f0) = 0;
    G32(0x109f13f4) = 0;
    for (bucket = 0; bucket < 0x3f1; bucket++) {
        uint32_t node = U32(G32(0x1015dc30), bucket * 4);

        while (node != 0) {
            if (S16(node, 2) == 0) {
                S16(node, 2) = 1;
                Sim_10027c70((void *)(uintptr_t)node);
            }
            node = U32(node, 8);
        }
    }
}

// FUNCTION: HGSIM 0x10027f33
void *__cdecl Sim_10027f33(int key, const void *name)
{
    int32_t sum;
    uint32_t node;

    if (key < 0)
        return 0;
    sum = (int32_t)((uint32_t)key + (uint32_t)(int32_t)(int8_t)U8(name, 0) + (uint32_t)(int32_t)(int8_t)U8(name, 1) +
                    (uint32_t)(int32_t)(int8_t)U8(name, 2) + (uint32_t)(int32_t)(int8_t)U8(name, 3));
    node = U32(G32(0x1015dc30), (uint32_t)(sum % 0x3f1) * 4u);
    while (node != 0) {
        if (U16(node, 0) == (uint32_t)key && U32(node, 4) == U32(name, 0))
            break;
        node = U32(node, 8);
    }
    return BYTES(node);
}

// FUNCTION: HGSIM 0x1002828e
void __cdecl Sim_1002828e(int key, const void *name)
{
    void *node;

    Sim_100d9fc0();
    node = Sim_10027f33(key, name);
    if (node != 0)
        Sim_10027c70(node);
}

// FUNCTION: HGSIM 0x10028a26
void __cdecl Sim_10028a26(void *obj, uint32_t value)
{
    U32(obj, 0xf4) = value;
}

// FUNCTION: HGSIM 0x1002c10e
void __cdecl Sim_1002c10e(void)
{
}
