#include "w2k.h"
#include "../util/access.h"

// FUNCTION: HGSIM 0x100aa190
void __cdecl Sim_100aa190(void *obj, const uint32_t *src)
{
    U32(obj, 0x3C) = src[0];
    U32(obj, 0x40) = src[1];
    U32(obj, 0x44) = src[2];
    U32(obj, 0x48) = src[3];
    U32(obj, 0x94) = U32(obj, 0x94) & ~0x4000u;
    U32(obj, 0x94) = U32(obj, 0x94) | 1;
}

// FUNCTION: HGSIM 0x100abb1b
void __cdecl Sim_100abb1b(void *node, uint32_t *head, uint32_t *tail)
{
    uint32_t prev, next;

    if (node == NULL || *head == 0 || *tail == 0)
        return;
    if (*head == (uint32_t)(uintptr_t)node) {
        if (*tail == (uint32_t)(uintptr_t)node) {
            *head = 0;
            *tail = 0;
        } else {
            *head = U32(node, 0x10);
            U32(*head, 0xC) = 0;
        }
    } else if (*tail == (uint32_t)(uintptr_t)node) {
        *tail = U32(node, 0xC);
        U32(*tail, 0x10) = 0;
    } else {
        prev = U32(node, 0xC);
        next = U32(node, 0x10);
        if (prev != 0)
            U32(prev, 0x10) = next;
        if (next != 0)
            U32(next, 0xC) = prev;
    }
    U32(node, 0xC) = 0;
    U32(node, 0x10) = 0;
}

// FUNCTION: HGSIM 0x100abbe8
void *__cdecl Sim_100abbe8(int index)
{
    void *entry = NULL;

    if (GS32(0x1017D1C4) != GS32(0x1017D1C8) - 1 && index >= 0 && index < GS32(0x1017D1C8)) {
        entry = GBYTES(0x101C57E0u + (uint32_t)index * 0x34u);
        G32(0x1017D1C4) = G32(0x1017D1C4) + 1;
        G32(0x101C56C0 + G32(0x1017D1C4) * 4) = (uint32_t)index;
        Sim_100abb1b(entry, SIM_PTR(uint32_t, 0x1017D1D0), SIM_PTR(uint32_t, 0x1017D1D4));
    }
    return entry;
}
