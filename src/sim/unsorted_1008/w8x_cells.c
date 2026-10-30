#include "w8x.h"
#include "w7n.h"
#include "../util/access.h"
#include "../util/util.h"
#include "../util/w3o.h"
#include "../view/view.h"
#include "../view/w2l.h"

#define W8X_ADDR(a) ((uint32_t)(uintptr_t)SIM_PTR(uint8_t, (a)))

/* Puts obj at the front of the list +0x10 of `cell` (prev +0xc, next +0x10), in the original's order. */
static void W8x_LinkFront(uint8_t *obj, uint32_t cell)
{
    if (U32(cell, 0x10) != 0)
        U32(U32(cell, 0x10), 0xc) = (uint32_t)(uintptr_t)obj;
    U32(obj, 0x10) = U32(cell, 0x10);
    U32(cell, 0x10) = (uint32_t)(uintptr_t)obj;
    U32(obj, 0xc) = cell;
    List_InsertBefore(obj, BYTES(cell));
}

/* The grid column / row of a coordinate: trunc(trunc((p + offset) / size) + 0.5). */
static int32_t W8x_Cell(uint32_t p, uint32_t offsetAddr)
{
    int32_t t = Render_Ftol(((double)Float_OfBits(p) + (double)GFL(offsetAddr)) / (double)GFL(0x1012b8e0));

    return Render_Ftol((double)t + (double)GFL(0x1012b8e4));
}

// FUNCTION: HGSIM 0x10089e91
void __cdecl Sim_10089e91(uint8_t *obj)
{
    uint8_t *list;
    uint32_t x, y, z;
    int32_t col, row;

    if (G32(0x1017c8cc) != W8X_ADDR(0x1017b988) || obj == 0)
        return;
    U32(obj, 0x8c) = 0x270f;
    U32(obj, 0x90) = 0x270f;
    if (G32(0x1017c8c4) != 0)
        list = Sim_10088804(obj);
    else
        list = (U16(obj, 0) & 0x1000) ? GBYTES(0x1017b9d8) : GBYTES(0x1017b988);
    Sim_1008a8f5(obj, list);
    if (S32(obj, 0x34) == 4)
        U16(obj, 0) = (uint16_t)(U16(obj, 0) | 0x800);
    if (U16(obj, 0) & 0x800)
        return;
    if (G32(0x1017c8c4) == 0) {
        W8x_LinkFront(obj, W8X_ADDR(0x1017b988));
        return;
    }
    if (U32(obj, 0x28) != 0) {
        Sim_100a9cfb(BYTES(U32(obj, 0x28)), &x, &y, &z);
    } else {
        x = U32(obj, 0x44);
        z = U32(obj, 0x4c);
    }
    col = W8x_Cell(x, 0x1014b8e0);
    row = W8x_Cell(z, 0x1014b8e4);
    if (col >= 0 && col < 9 && row >= 0 && row < 9)
        W8x_LinkFront(obj, W8X_ADDR(0x1017ba28) + (uint32_t)col * 0x18c + (uint32_t)row * 0x2c);
    else {
        /* the cell [0x1017c8c8], read again at each step like the original does */
        if (U32(G32(0x1017c8c8), 0x10) != 0)
            U32(U32(G32(0x1017c8c8), 0x10), 0xc) = (uint32_t)(uintptr_t)obj;
        U32(obj, 0x10) = U32(G32(0x1017c8c8), 0x10);
        U32(G32(0x1017c8c8), 0x10) = (uint32_t)(uintptr_t)obj;
        U32(obj, 0xc) = G32(0x1017c8c8);
        List_InsertBefore(obj, BYTES(G32(0x1017c8c8)));
    }
}
