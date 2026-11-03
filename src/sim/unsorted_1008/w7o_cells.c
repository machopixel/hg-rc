#include "w7o.h"
#include "w6f.h"
#include "../util/access.h"
#include "../util/w3o.h"
#include "../util/util.h"

/* trunc(trunc((v + [offAddr]) / [0x1012b8e0]) + [0x1012b8e4]): the grid index of a coordinate. */
static int32_t CellIndex(uint32_t v, uint32_t offAddr)
{
    int32_t t = Render_Ftol(((double)Float_OfBits(v) + (double)GFL(offAddr)) / (double)GFL(0x1012b8e0));

    return Render_Ftol((double)t + (double)GFL(0x1012b8e4));
}

// FUNCTION: HGSIM 0x10088a2d
void __cdecl Sim_10088a2d(uint16_t *obj)
{
    uint32_t x, y, z;
    int32_t column, row;

    if (obj == 0 || G32(0x1017c8c4) == 0 || (obj[0] & 0x1000) != 0)
        return;
    Sim_100a9cfb((void *)(uintptr_t)U32(obj, 0x28), &x, &y, &z);
    column = CellIndex(x, 0x1014b8e0);
    row = CellIndex(z, 0x1014b8e4);
    if (column == S32(obj, 0x8c) && row == S32(obj, 0x90))
        return;
    if (column >= 0 && column < 9 && row >= 0 && row < 9)
        Sim_10088b24((void *)(uintptr_t)U32(obj, 0x28), column, row);
    else
        Sim_10088986((void *)(uintptr_t)U32(obj, 0x28));
}

// FUNCTION: HGSIM 0x1008d4f1
void __cdecl Sim_1008d4f1(int32_t *bmp, int32_t colour)
{
    int32_t w = (int32_t)((uint32_t)bmp[3] - (uint32_t)bmp[1]);
    int32_t h = (int32_t)((uint32_t)bmp[4] - (uint32_t)bmp[2]);

#define SIDE(x0, y0, x1, y1)                                                                              \
    do {                                                                                                  \
        W7O_CALL_100ef727(bmp, x0, y0, x1, y1, 0, SIM_PTR(uint16_t, 0x101659e0)[colour]);                 \
    } while (0)
    SIDE(0, 0, w, 0);
    SIDE(w, 0, w, h);
    SIDE(w, h, 0, h);
    SIDE(0, h, 0, 0);
#undef SIDE
}
