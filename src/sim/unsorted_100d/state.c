#include "fns.h"
#include "../simmem.h"

#define U32(p) (*(uint32_t *)(uintptr_t)(p))
/* The camera object the view code works on (pointer stored in the DLL's data). */
#define CURRENT_VIEW ((uint8_t *)(uintptr_t)*SIM_PTR(uint32_t, 0x1014cae0))

// FUNCTION: HGSIM 0x100d1cce
void __cdecl Sim_100d1cce(void)
{
    *SIM_PTR(uint32_t, 0x10151f8c) = *SIM_PTR(uint32_t, 0x101c457c);
    *SIM_PTR(uint32_t, 0x10151fa4) = *SIM_PTR(uint32_t, 0x101c49ac);
    *SIM_PTR(uint32_t, 0x10151fb8) = *SIM_PTR(uint32_t, 0x101c47c8);
    *SIM_PTR(uint32_t, 0x10151fcc) = *SIM_PTR(uint32_t, 0x101c4854);
    *SIM_PTR(uint32_t, 0x10151fe0) = *SIM_PTR(uint32_t, 0x101c49e8);
    *SIM_PTR(uint32_t, 0x10151ff4) = *SIM_PTR(uint32_t, 0x101c4680);
}

// FUNCTION: HGSIM 0x100d252c
void __cdecl Sim_100d252c(void *object, uint32_t value)
{
    *SIM_PTR(uint32_t, 0x10185c50) = value;
    U32((uint8_t *)object + 0x3c) = *SIM_PTR(uint32_t, 0x10185c50);
}

// FUNCTION: HGSIM 0x100d2545
void __cdecl Sim_100d2545(void *object, uint32_t value)
{
    *SIM_PTR(uint32_t, 0x10185bf8) = value;
    U32((uint8_t *)object + 0x40) = *SIM_PTR(uint32_t, 0x10185bf8);
}

/*
 * Table of 11 five-dword records at 0x10185400. Selecting record n zeroes
 * four fields of the current view, stores the record's (+0xc - +4) and
 * (+0x10 - +8) differences in it, and copies the record to 0x109e3e00.
 */
// FUNCTION: HGSIM 0x100d32e5
void __cdecl Sim_100d32e5(int preset)
{
    uint32_t *rec;
    uint8_t *view;
    int i;

    if (preset < 0 || preset >= 0xb)
        return;
    if (preset == *SIM_PTR(int32_t, 0x1015241c))
        return;
    rec = SIM_PTR(uint32_t, 0x10185400 + (uint32_t)preset * 0x14);
    view = CURRENT_VIEW;
    U32(view + 0x2c) = 0;
    U32(view + 0x34) = 0;
    U32(view + 0x30) = rec[3] - rec[1];
    U32(view + 0x38) = rec[4] - rec[2];
    U32(view + 0x4c) = 0;
    U32(view + 0x50) = 0;
    for (i = 0; i < 5; i++)
        SIM_PTR(uint32_t, 0x109e3e00)[i] = rec[i];
    *SIM_PTR(int32_t, 0x1015241c) = preset;
    *SIM_PTR(int32_t, 0x1013a014) = 1;
}

// FUNCTION: HGSIM 0x100d4500
void __cdecl Sim_100d4500(void *object)
{
    uint8_t *o = object;
    int i;

    for (i = 0; i < 8; i++) {
        uint8_t *entry = (uint8_t *)(uintptr_t)(U32(o + 0x84) + (uint32_t)i * 0x128);

        if (U32(entry + 8) == 2 || U32(entry + 8) == 3)
            U32(entry + 8) = 1;
    }
    for (i = 0; i < 8; i++) {
        U32(o + (uint32_t)i * 4 + 0xbf5) = 0; /* not 4-aligned */
        *SIM_PTR(uint32_t, 0x101853c0 + (uint32_t)i * 4) = 0;
    }
}

// FUNCTION: HGSIM 0x100d6392
void __cdecl Sim_100d6392(uint32_t value)
{
    uint8_t *a = (uint8_t *)(uintptr_t)*SIM_PTR(uint32_t, 0x109e8b40 + *SIM_PTR(uint32_t, 0x1017b8c8) * 4);
    uint8_t *b = (uint8_t *)(uintptr_t)U32(a + 0x28);
    uint8_t *row = (uint8_t *)(uintptr_t)(U32(b + 0x84) + U32(b + 0x4fc) * 0x128);

    U32(row + 0xc) = value;
}

/* Adds the offset in 0x1017e6a8; the setter is Sim_100db77e. */
// FUNCTION: HGSIM 0x100db770
int __cdecl Sim_100db770(int value)
{
    return (int)((uint32_t)value + *SIM_PTR(uint32_t, 0x1017e6a8));
}

// FUNCTION: HGSIM 0x100db77e
void __cdecl Sim_100db77e(int value)
{
    *SIM_PTR(int32_t, 0x1017e6a8) = value;
}

// FUNCTION: HGSIM 0x100e282a
void __cdecl Sim_100e282a(uint32_t value)
{
    *SIM_PTR(uint32_t, 0x1017e850) = value;
}
