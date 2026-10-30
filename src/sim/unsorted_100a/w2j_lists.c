#include "w2j.h"
#include "../util/access.h"

// FUNCTION: HGSIM 0x100aac3d
void *__cdecl Sim_100aac3d(const void *obj, int n)
{
    uint32_t node;
    int32_t count = 0;
    int32_t steps;

    if (obj == 0)
        return 0;
    node = U32(obj, 4);
    if (node == 0)
        return 0;
    while (node != 0) {
        count++;
        node = U32(node, 8);
    }
    steps = (int32_t)((uint32_t)count - (uint32_t)n - 1u);
    if (steps < 0)
        return 0;
    node = U32(obj, 4);
    while (steps-- != 0 && node != 0)
        node = U32(node, 8);
    return BYTES(node);
}

// FUNCTION: HGSIM 0x100abab0
void __cdecl Sim_100abab0(void *node, void *head, void *tail)
{
    if (node == 0)
        return;
    if (U32(head, 0) == 0 && U32(tail, 0) == 0) {
        U32(node, 0xc) = 0;
        U32(node, 0x10) = 0;
        U32(head, 0) = (uint32_t)(uintptr_t)node;
        U32(tail, 0) = (uint32_t)(uintptr_t)node;
        return;
    }
    U32(node, 0xc) = U32(tail, 0);
    U32(node, 0x10) = 0;
    U32(U32(tail, 0), 0x10) = (uint32_t)(uintptr_t)node;
    U32(tail, 0) = (uint32_t)(uintptr_t)node;
}

// FUNCTION: HGSIM 0x100abc5d
void *__cdecl Sim_100abc5d(void)
{
    uint8_t *node = 0;

    if (GS32(0x1017d1c4) >= 0) {
        node = SIM_PTR(uint8_t, 0x101c57e0u + SIM_PTR(u32a, 0x101c56c0)[GS32(0x1017d1c4)] * 0x34u);
        Sim_100abab0(node, SIM_PTR(uint8_t, 0x1017d1d0), SIM_PTR(uint8_t, 0x1017d1d4));
        G32(0x1017d1c4) = G32(0x1017d1c4) - 1;
    }
    return node;
}
