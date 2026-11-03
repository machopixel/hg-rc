#include "w5a.h"
#include "../util/access.h"

// FUNCTION: HGSIM 0x100a2b8c
void __cdecl Sim_100a2b8c(uint32_t a, uint32_t b, uint32_t c)
{
    uint32_t table = (uint32_t)(uintptr_t)SIM_PTR(void, 0x109e3e00);

    W5A_CALL_100ef727(table, a, b, G32(0x101cc2bc), G32(0x101ccc6c), 0, SIM_PTR(u16a, 0x101659e0)[c]);
    G32(0x101cc2bc) = a;
    G32(0x101ccc6c) = b;
}

// FUNCTION: HGSIM 0x100a796c
int __cdecl File_ReadChunked(uint32_t handle, uint32_t buffer, uint32_t count)
{
    uint32_t left = count;
    uint32_t at = buffer;
    uint32_t chunk;

    while (left != 0) {
        chunk = left < 0x4000 ? left : 0x4000;
        if (W5A_CALL_10105b50(handle, at, chunk) != (int)chunk)
            return -1;
        left -= chunk;
        at += chunk;
    }
    return (int)count;
}

// FUNCTION: HGSIM 0x100aa5e8
void __cdecl Sim_100aa5e8(void *node, uint32_t mask)
{
    uint32_t child;

    if (U32(node, 0x8c) != 0)
        U16(U32(node, 0x8c), 2) = (uint16_t)(U16(U32(node, 0x8c), 2) & ~mask);
    for (child = U32(node, 4); child != 0; child = U32(child, 8))
        Sim_100aa5e8((void *)(uintptr_t)child, mask);
}

// FUNCTION: HGSIM 0x100aa742
void __cdecl Sim_100aa742(void *node)
{
    uint32_t child;

    if ((U32(node, 0x94) & 1) != 0) {
        W5A_CALL_100aa64a(node);
        return;
    }
    if (U32(node, 4) == 0)
        return;
    for (child = U32(node, 4); child != 0; child = U32(child, 8))
        Sim_100aa742((void *)(uintptr_t)child);
}

// FUNCTION: HGSIM 0x100aacd5
void *__cdecl Sim_100aacd5(void *node, uint32_t id)
{
    uint32_t child;
    void *found;

    if (U32(node, 0x8c) != 0 && U16(U32(node, 0x8c), 0x26) == id)
        return node;
    for (child = U32(node, 4); child != 0; child = U32(child, 8)) {
        found = Sim_100aacd5((void *)(uintptr_t)child, id);
        if (found != 0)
            return found;
    }
    return 0;
}
