#include "w3q.h"
#include "../unsorted_100a/w2h.h"
#include "../util/access.h"

// FUNCTION: HGSIM 0x100a5c10
void *__cdecl Sim_100a5c10(void *obj, const uint32_t *screen)
{
    uint8_t *v;

    if (screen == 0 && U32(obj, 0x2c) != 0)
        return (void *)(uintptr_t)U32(obj, 0x2c);
    v = BYTES(Sim_100a88ff());
    U32(obj, 0x2c) = (uint32_t)(uintptr_t)v;
    U32(v, 0) = U32(obj, 0x20);
    U32(v, 4) = U32(obj, 0x24);
    U32(v, 8) = U32(obj, 0x28);
    U32(v, 0xc) = U32(obj, 0);
    U32(v, 0x10) = U32(obj, 4);
    U32(v, 0x14) = U32(obj, 8);
    if (screen != 0) {
        U32(v, 0x20) = screen[0];
        U32(v, 0x24) = screen[1];
    } else {
        U32(v, 0x20) = U32(obj, 0x18);
        U32(v, 0x24) = U32(obj, 0x1c);
    }
    U32(v, 0x30) = U32(obj, 0x40);
    U32(v, 0x34) = U32(obj, 0x44);
    U32(v, 0x38) = U32(obj, 0x48);
    U32(v, 0x2c) = U32(obj, 0x10);
    return v;
}

// FUNCTION: HGSIM 0x100a5d25
void *__cdecl Sim_100a5d25(void *obj)
{
    return Sim_100a5c10(obj, 0);
}
