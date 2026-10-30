#include "w3q.h"
#include "../util/util.h"
#include "../util/w3q.h"

// FUNCTION: HGSIM 0x1000951c
void __cdecl Sim_1000951c(const void *list, void(__cdecl *fn)(void *node))
{
    uint32_t node;

    if (list == 0)
        return;
    node = U32(list, 8);
    while (node != 0) {
        uint32_t next = U32(node, 8);

        fn((void *)(uintptr_t)node);
        node = next;
    }
}

// FUNCTION: HGSIM 0x10008e46
void __cdecl Sim_10008e46(void *obj, float limit)
{
    uint32_t node;

    for (node = U32(obj, 0x2c); node != 0; node = U32(node, 0xc)) {
        if (X87_NOT_ABOVE(F32A(node, 0), limit)) {
            Sim_10008c69(obj, node);
            return;
        }
    }
}

// FUNCTION: HGSIM 0x1000ca63
int __cdecl Sim_1000ca63(int kind, float value, float *out)
{
    int i;
    int count = GS32(0x1015d0a0);
    const uint8_t *rec = GBYTES(0x10a3a600);

    for (i = 0; i < count; i++, rec += 0xa8) {
        int slot;

        if ((uint32_t)kind != U32(rec, 0))
            continue;
        for (slot = 0; slot < 0x14; slot++) {
            if (X87_BELOW(F32A(rec, 8 + slot * 4), value))
                continue;
            slot--;
            if (slot < 0)
                slot = 0;
            U32(out, 0) = U32(rec, 0x58 + slot * 4);
            if (G32(0x10145518 + (uint32_t)kind * 0xd6) == 0x45) {
                F32A(out, 0) = (float)((double)Sim_1009c929(10) + (double)F32A(out, 0));
                if (X87_ABOVE(F32A(out, 0), GF32(0x10111098)))
                    U32(out, 0) = 0x42340000;
            }
            return 1;
        }
        slot--;
        U32(out, 0) = U32(rec, 0x58 + slot * 4);
        return 1;
    }
    /* Not found: the original returns whatever its registers held, not 0: the count when it is not
     * positive, else the address just behind the last record. Kept because eax is visible to callers. */
    if (count <= 0)
        return count;
    return (int)(uintptr_t)rec;
}
