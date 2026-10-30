#include "w2d.h"
#include "../util/util.h"
#include "../util/access.h"
#include "../unsorted_1003/fns.h"

// FUNCTION: HGSIM 0x100d709d
void __cdecl Sim_100d709d(void *obj, uint32_t *a, uint32_t *b, uint32_t *c, void *q)
{
    uint32_t block = U32(obj, 0x28);

    if (S32(q, 0) != -1) {
        uint32_t target = U32(block, 0x498 + U32(q, 0x11c) * 4);

        if (target != 0)
            Sim_100a9cfb((void *)(uintptr_t)target, a, b, c);
    } else {
        Sim_100a9cfb((void *)(uintptr_t)U32(obj, 0x4c), a, b, c);
    }
}

// FUNCTION: HGSIM 0x100dc286
uint32_t __cdecl Sim_100dc286(void)
{
    uint32_t result;

    if (GS32(0x1017E6A4) < GS32(0x1017E6A0)) {
        result = Sim_1003b417((int)G32(0x1017EEC0 + G32(0x1017E6A4) * 4));
        G32(0x1017E6A4) = G32(0x1017E6A4) + 1;
    } else {
        result = 0;
    }
    return result;
}
