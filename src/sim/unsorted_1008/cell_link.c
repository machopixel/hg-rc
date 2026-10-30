#include "fns.h"
#include "../simmem.h"
#include "../view/view.h"

/* Field of a structure at a byte offset. */
#define FLD(type, p, off) (*(type *)((uint8_t *)(p) + (off)))
/* Pointers are stored as 32-bit values in the DLL's data. */
#define PTR(v) ((uint8_t *)(uintptr_t)(v))
#define U32(p) ((uint32_t)(uintptr_t)(p))

// FUNCTION: HGSIM 0x1008a220
void __cdecl Sim_1008a220(uint16_t *obj)
{
    uint8_t *cell;
    uint8_t *head;

    if (obj == 0)
        return;
    if ((*obj & 0x800) == 0 || FLD(int32_t, obj, 0x34) == 4)
        return;
    *obj &= 0xf7ff;
    if (FLD(int32_t, obj, 4) == 0)
        return;
    if (*SIM_PTR(int32_t, 0x1017c8cc) == 0)
        return;
    if (*SIM_PTR(int32_t, 0x1017c8c4) != 0) {
        int32_t x = FLD(int32_t, obj, 0x8c);
        int32_t y = FLD(int32_t, obj, 0x90);

        if (x >= 0 && x < 9 && y >= 0 && y < 9)
            cell = (uint8_t *)SIM_PTR(uint8_t, 0x1017ba28 + (uint32_t)x * 0x18c + (uint32_t)y * 0x2c);
        else
            cell = (uint8_t *)SIM_PTR(uint8_t, 0x1017b9b0);
    } else {
        cell = PTR(*SIM_PTR(uint32_t, 0x1017c8cc));
    }
    if (FLD(uint32_t, cell, 0x10) != 0)
        FLD(uint32_t, PTR(FLD(uint32_t, cell, 0x10)), 0xc) = U32(obj);
    FLD(uint32_t, obj, 0x10) = FLD(uint32_t, cell, 0x10);
    FLD(uint32_t, cell, 0x10) = U32(obj);
    FLD(uint32_t, obj, 0xc) = U32(cell);
    List_InsertBefore(obj, cell);
}
