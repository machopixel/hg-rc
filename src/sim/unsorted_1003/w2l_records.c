#include "w2l.h"
#include "../util/access.h"
#include "../unsorted_1000/w2l.h"

// FUNCTION: HGSIM 0x1003b8e3
void __cdecl Sim_1003b8e3(int key)
{
    int i, j;

    for (i = 0; i < GS32(0x1015ea80); i++) {
        uint8_t *rec = SIM_PTR(uint8_t, 0x109b9e20u + (uint32_t)i * 0x7e);

        if (S32(rec, 0x60) != key)
            continue;
        for (j = 0; j < 4; j++) {
            if (U32(rec, 0x50 + j * 4) != 0)
                Sim_10009787((void *)(uintptr_t)U32(rec, 0x50 + j * 4));
            U32(rec, 0x50 + j * 4) = 0;
        }
        S32(rec, 4) = -1;
        U16(rec, 0x7c) = 1;
        return;
    }
}

// FUNCTION: HGSIM 0x1003db30
void __cdecl Sim_1003db30(int index)
{
    uint32_t entry = PTRTAB(index);

    /* The original loads the field and never uses it; the read is kept so a bad entry faults as before. */
    if (entry != 0)
        (void)*(volatile u32a *)(BYTES(entry) + 0x28);
}
