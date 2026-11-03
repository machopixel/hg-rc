#include "w4u.h"
#include "../../platform/plat.h"
#include "w2e.h"
#include "../win/w4u_api.h"

// FUNCTION: HGSIM 0x10030150
void *__cdecl ListNode_Alloc(void)
{
    return (void *)(uintptr_t)W4u_HeapAlloc(8);
}

// FUNCTION: HGSIM 0x10030175
void __cdecl List_FreeNodes(void *owner)
{
    uint32_t node;
    uint32_t next;

    if (owner == 0 || U32(owner, 0x1c) == 0)
        return;
    node = U32(owner, 0x1c);
    while (node != 0) {
        next = U32(node, 0);
        W4u_HeapFree(node);
        node = next;
    }
    U32(owner, 0x1c) = 0;
}

// FUNCTION: HGSIM 0x100301cc
void *__cdecl List_PushFront(void *owner, void *value)
{
    uint32_t node = 0;

    if (owner == 0 || value == 0)
        return 0;
    node = (uint32_t)(uintptr_t)ListNode_Alloc();
    if (node != 0) {
        U32(node, 4) = (uint32_t)(uintptr_t)value;
        U32(node, 0) = U32(owner, 0x1c);
        U32(owner, 0x1c) = node;
        U32(owner, 0x18) = U32(owner, 0x18) + 1;
    }
    return (void *)(uintptr_t)node;
}

// FUNCTION: HGSIM 0x1003058f
void *__cdecl Sim_1003058f(void)
{
    uint32_t block = W4u_HeapAlloc(0x644);

    if (block != 0) {
        U32(block, 0x10) = 0x7f7fc99e;
        U32(block, 8) = 0x7f7fc99e;
        U32(block, 0) = 0x7f7fc99e;
        U32(block, 0x14) = 0xff7fc99e;
        U32(block, 0xc) = 0xff7fc99e;
        U32(block, 4) = 0xff7fc99e;
    }
    return (void *)(uintptr_t)block;
}

// FUNCTION: HGSIM 0x100305f8
void __cdecl Sim_100305f8(void *owner)
{
    int row;
    int col;

    if (owner == 0)
        return;
    for (row = 0; row < 7; row++) {
        for (col = 0; col < 7; col++)
            List_FreeNodes(BYTES(owner) + 0x24 + (uint32_t)row * 0xe0 + ((uint32_t)col << 5));
    }
    W4u_HeapFree((uint32_t)(uintptr_t)owner);
}

// FUNCTION: HGSIM 0x1003693c
int __cdecl Sim_1003693c(void)
{
    if (G32(0x1015e570) != 0)
        Plat_FreeLibrary(*SIM_PTR(void *, 0x1015e568));     /* FreeLibrary: the platform's (built-in renderer) */
    return 1;
}

// FUNCTION: HGSIM 0x1003913b
void __cdecl Sim_1003913b(uint32_t key, void *other)
{
    uint32_t rec = G32(0x109e3760);

    while (rec != 0) {
        if (U32(rec, 0) == key) {
            if (U32(rec, 0x690) != 0)
                U32(U32(rec, 0x690), 0x694) = U32(rec, 0x694);
            else
                G32(0x109e3770) = U32(rec, 0x694);
            if (U32(rec, 0x694) != 0)
                U32(U32(rec, 0x694), 0x690) = U32(rec, 0x690);
            else
                G32(0x109e3760) = U32(rec, 0x690);
            Sim_10038ad5(other, BYTES(rec));
            W4u_HeapFree(rec);
            return;
        }
        rec = U32(rec, 0x690);
    }
}
