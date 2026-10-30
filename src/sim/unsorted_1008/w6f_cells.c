#include "w6f.h"
#include "../util/access.h"
#include "../view/w2l.h"
#include "../unsorted_1000/w5z.h"

#define ADDR(p) ((uint32_t)(uintptr_t)(p))
#define PTR(v) ((void *)(uintptr_t)(v))

// FUNCTION: HGSIM 0x10088986
void __cdecl Sim_10088986(void *node)
{
    uint32_t child;

    if (node == 0 || G32(0x1017c8c4) == 0)
        return;
    if (U32(node, 0x8c) != 0) {
        Sim_1008a871(PTR(U32(node, 0x8c)));
        Sim_1008a8f5(PTR(U32(node, 0x8c)), SIM_PTR(void, 0x1017b9b0));
        U32(U32(node, 0x8c), 0x8c) = 9999;
        U32(U32(node, 0x8c), 0x90) = 9999;
    }
    for (child = U32(node, 4); child != 0; child = U32(child, 8))
        Sim_10088986(PTR(child));
}

// FUNCTION: HGSIM 0x10088b24
void __cdecl Sim_10088b24(void *node, int32_t row, int32_t col)
{
    uint32_t child;

    if (node == 0 || G32(0x1017c8c4) == 0)
        return;
    if (U32(node, 0x8c) == 0)
        return;
    Sim_1008a871(PTR(U32(node, 0x8c)));
    Sim_1008a8f5(PTR(U32(node, 0x8c)),
                 PTR(ADDR(SIM_PTR(uint8_t, 0x1017ba28)) + (uint32_t)row * 0x18cu + (uint32_t)col * 0x2cu));
    U32(U32(node, 0x8c), 0x8c) = (uint32_t)row;
    U32(U32(node, 0x8c), 0x90) = (uint32_t)col;
    for (child = U32(node, 4); child != 0; child = U32(child, 8)) {
        uint32_t obj = U32(child, 0x8c);

        if (obj == 0)
            continue;
        if ((U16(U32(child, 0x8c), 0) & 0x1000) != 0 && (U16(U32(child, 0x8c), 2) & 0xf0) == 0x70)
            continue;
        Sim_10088b24(PTR(child), row, col);
    }
}

/* Frees the objects of the list whose head is the dword at `head` until the head stays in place. */
static void FreeCellList(u32a *head)
{
    int stuck = 0;

    while (*head != 0 && stuck == 0) {
        uint32_t obj = *head;

        Sim_1008a16d(PTR(obj));
        stuck = obj == *head;
        Sim_10009369(PTR(obj));
    }
}

// FUNCTION: HGSIM 0x1008a5e3
void __cdecl Sim_1008a5e3(void)
{
    int32_t row, col;

    if (G32(0x1017c8c4) != 0) {
        for (row = 0; row < 9; row++)
            for (col = 0; col < 9; col++)
                FreeCellList(SIM_PTR(u32a, 0x1017ba30 + (uint32_t)row * 0x18cu + (uint32_t)col * 0x2cu));
        FreeCellList(SIM_PTR(u32a, 0x1017b9b8));
    } else if (G32(0x1017c8cc) == ADDR(SIM_PTR(uint8_t, 0x1017b988))) {
        FreeCellList(SIM_PTR(u32a, 0x1017b990));
    }
    if (G32(0x1017c8cc) == ADDR(SIM_PTR(uint8_t, 0x1017b988))) {
        FreeCellList(SIM_PTR(u32a, 0x1017b9e0));
        FreeCellList(SIM_PTR(u32a, 0x1017c820));
        FreeCellList(SIM_PTR(u32a, 0x1017ba08));
    }
}
