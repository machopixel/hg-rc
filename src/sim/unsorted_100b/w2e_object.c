#include "w2e.h"
#include "../unsorted_100f/w2e.h"
#include "../util/access.h"

/* Sets or clears bit `bit` of the dword at obj+0x26c. */
static void SetFlag(void *obj, int bit, int on)
{
    U32(obj, 0x26c) = (U32(obj, 0x26c) & ~(1u << bit)) | ((uint32_t)(on != 0) << bit);
}

// FUNCTION: HGSIM 0x100bea42
void __cdecl Sim_100bea42(void *obj)
{
    U16(obj, 0x1f8) = 0xffff;
    U16(obj, 0x1fa) = 0xffff;
    U32(obj, 0x1fc) = 0xFFFFFFFFu;
    U32(obj, 0x24a) = 0;
    U16(obj, 0x25e) = 0;
    U32(obj, 0x260) = 0x3f800000;
    U32(obj, 0x25a) = 0;
    Mem_Fill(BYTES(obj) + 0x270, 0, 8);

    if ((int8_t)U8(obj, 0x1b7) < 1)
        U8(obj, 0x1b7) = 1;
    if ((int8_t)U8(obj, 0x1b7) > 6)
        U8(obj, 0x1b7) = 6;

    SetFlag(obj, 0, (int8_t)U8(obj, 0x1b7) <= 3);
    SetFlag(obj, 1, (int8_t)U8(obj, 0x1b7) <= 3);
    SetFlag(obj, 2, (int8_t)U8(obj, 0x1b7) <= 2);
    SetFlag(obj, 3, U32(obj, 0) == 1 && (int8_t)U8(obj, 0x1b7) <= 5);
    SetFlag(obj, 4, (int8_t)U8(obj, 0x1b7) <= 4);
    SetFlag(obj, 5, U32(obj, 0) == 1 && (int8_t)U8(obj, 0x1b7) <= 1);
    SetFlag(obj, 6, (int8_t)U8(obj, 0x1b7) <= 5);

    if (U32(obj, 0) == 1 && ((U32(obj, 0x26c) >> 3) & 1) == 0)
        U32(U32(obj, 0x28), 0x534) = 0;
    if (U32(obj, 0x27c) == 0) {
        U32(obj, 0x27c) = (uint32_t)(uintptr_t)GBYTES(0x10151ae0) + U32(obj, 0) * 8;
        if (U32(U32(obj, 0x28), 0x538) == 1)
            U32(obj, 0x27c) = (uint32_t)(uintptr_t)GBYTES(0x10151b38);
    }
    if (U32(obj, 0x278) == 0)
        U32(obj, 0x278) = 0x3a98;
    U32(obj, 0x23a) = 0xFFFFFFFFu;
}
