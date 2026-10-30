#include "w7q.h"
#include "w2i.h"
#include "../util/access.h"

#define W7Q_RECORD(block, i) (U32(block, 0x84) + (uint32_t)(i) * 0x128u)
#define W7Q_TYPE(n) ((uint32_t)(uintptr_t)SIM_PTR(uint8_t, 0x10145508u + (uint32_t)(n) * 0xd6u))

/* The set-up of one record in state 1 (the common body of Sim_100d650f and Sim_100d666d). */
static void W7q_SetUpRecord(uint8_t *block, uint32_t rec, uint32_t type)
{
    if (S32(type, 0x10) == 0x3d && S32(rec, 0x20) != -2)
        U16(block, 0x562) = (uint16_t)(U16(block, 0x562) | 0x800u);
    else
        U32(rec, 8) = 2;
    U32(rec, 0x2c) = U32(type, 0x2c);
    U32(rec, 0x10) = 0;
    W7Q_CALL_100c398d((void *)(uintptr_t)rec, block);
    if ((U32(type, 0x86) & 1u) != 0 && (U16(block, 0x560) & 0x80u) != 0) {
        U32(rec, 0x18) = U32(U32(block, 0), 0xf0) & 0xffu;
        U32(rec, 0x1c) = U32(U32(block, 0), 0xf0) & 0xf00u;
    } else {
        U32(rec, 0x1c) = 0;
        U32(rec, 0x18) = 0;
    }
}

// FUNCTION: HGSIM 0x100d650f
void __cdecl Sim_100d650f(void *block)
{
    uint8_t *b = block;
    uint32_t saved = U32(b, 0x4fc);
    uint32_t first;

    Sim_100d5cc1(b, 0);
    first = U32(b, 0x4fc);
    do {
        uint32_t rec = W7Q_RECORD(b, U32(b, 0x4fc));
        uint32_t type = W7Q_TYPE(U32(rec, 4));

        if (S32(rec, 8) == 1)
            W7q_SetUpRecord(b, rec, type);
        Sim_100d5cc1(b, 0);
    } while (U32(b, 0x4fc) != first);
    U32(b, 0x4fc) = saved;
}

// FUNCTION: HGSIM 0x100d666d
void __cdecl Sim_100d666d(void *block)
{
    uint8_t *b = block;
    int32_t count;
    int32_t i;

    if (b == 0)
        return;
    count = S32(b, 0x4f8);
    for (i = 0; i < count; i++) {
        uint32_t rec = W7Q_RECORD(b, i);
        uint32_t type = W7Q_TYPE(U32(rec, 4));

        if (S32(rec, 8) == 1)
            W7q_SetUpRecord(b, rec, type);
    }
}
