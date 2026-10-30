#include "w2l.h"
#include "view.h"
#include "../util/access.h"

/* Pointers are stored as 32-bit values in the DLL's data. */
#define ADDR(p) ((uint32_t)(uintptr_t)(p))
#define CELL_HEAD ADDR(SIM_PTR(uint8_t, 0x1017ba00))
#define SPARE_CELL ADDR(SIM_PTR(uint8_t, 0x1017b9d8))
#define OTHER_CELL ADDR(SIM_PTR(uint8_t, 0x1017b988))
#define LAST_CELL ADDR(SIM_PTR(uint8_t, 0x1017c818))

// FUNCTION: HGSIM 0x10088d74
void __cdecl ObjList2_InsertAfter(void *node, void *after)
{
    uint32_t owner;

    if (node == 0 || after == 0 || U32(node, 0x20) != 0 || U32(node, 0x1c) != 0)
        return;
    owner = U32(node, 0x28);
    if (owner != 0 && U32(owner, 0) != 0)
        return;
    if (U32(after, 0x20) != 0)
        U32(U32(after, 0x20), 0x1c) = ADDR(node);
    U32(node, 0x20) = U32(after, 0x20);
    U32(after, 0x20) = ADDR(node);
    U32(node, 0x1c) = ADDR(after);
}

// FUNCTION: HGSIM 0x10088de6
void __cdecl ObjList2_Unlink(void *node)
{
    if (node == 0)
        return;
    if (U32(node, 0x20) != 0)
        U32(U32(node, 0x20), 0x1c) = U32(node, 0x1c);
    if (U32(node, 0x1c) != 0)
        U32(U32(node, 0x1c), 0x20) = U32(node, 0x20);
    U32(node, 0x1c) = 0;
    U32(node, 0x20) = 0;
}

// FUNCTION: HGSIM 0x1008a871
void __cdecl Sim_1008a871(void *node)
{
    if (U32(node, 8) != 0)
        U32(U32(node, 8), 4) = U32(node, 4);
    if (U32(node, 4) != 0)
        U32(U32(node, 4), 8) = U32(node, 8);
    U32(node, 4) = 0;
    U32(node, 8) = 0;
    if (G32(0x1017c8c4) != 0) {
        U32(node, 0x8c) = 9999;
        U32(node, 0x90) = 9999;
    }
    ObjList2_Unlink(node);
    ObjList_Unlink(node);
}

// FUNCTION: HGSIM 0x1008a8f5
void __cdecl Sim_1008a8f5(void *node, void *anchor)
{
    if (U32(anchor, 8) != 0)
        U32(U32(anchor, 8), 4) = ADDR(node);
    U32(node, 8) = U32(anchor, 8);
    U32(anchor, 8) = ADDR(node);
    U32(node, 4) = ADDR(anchor);
    ObjList2_InsertAfter(node, anchor);
    List_InsertBefore(node, anchor);
}

// FUNCTION: HGSIM 0x1008a14f
void __cdecl Sim_1008a14f(void *node)
{
    if (node != 0)
        Sim_1008a8f5(node, (void *)(uintptr_t)CELL_HEAD);
}

// FUNCTION: HGSIM 0x1008a16d
void __cdecl Sim_1008a16d(void *node)
{
    if (node == 0 || ADDR(node) == G32(0x1017c8cc) || ADDR(node) == SPARE_CELL)
        return;
    Sim_1008a871(node);
    if (U32(node, 0x10) != 0)
        U32(U32(node, 0x10), 0xc) = U32(node, 0xc);
    if (U32(node, 0xc) != 0)
        U32(U32(node, 0xc), 0x10) = U32(node, 0x10);
    U32(node, 0xc) = 0;
    U32(node, 0x10) = 0;
    ObjList_Unlink(node);
}

// FUNCTION: HGSIM 0x1008a1ea
void __cdecl Sim_1008a1ea(void *node)
{
    if (G32(0x1017c8cc) != OTHER_CELL || node == 0)
        return;
    Sim_1008a871(node);
    Sim_1008a8f5(node, (void *)(uintptr_t)LAST_CELL);
}

// FUNCTION: HGSIM 0x1008a40a
void __cdecl Sim_1008a40a(void *node)
{
    if (node == 0 || (U16(node, 0) & 0x1000) != 0)
        return;
    ObjList_Unlink(node);
    U16(node, 0) = (uint16_t)(U16(node, 0) | 0x1000);
    if (U32(node, 4) == 0)
        return;
    Sim_1008a871(node);
    Sim_1008a8f5(node, (void *)(uintptr_t)SPARE_CELL);
}
