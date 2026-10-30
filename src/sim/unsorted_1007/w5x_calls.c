#include "w5x.h"
#include "../unsorted_100a/w5x.h"
#include "../util/access.h"

// FUNCTION: HGSIM 0x10072d16
void __cdecl Sim_10072d16(void *p)
{
    if (p != 0)
        SIM_CALL_1002c1f9(p);
}

// FUNCTION: HGSIM 0x10072d3a
void __cdecl Sim_10072d3a(void *p)
{
    if (p != 0) {
        if (U32(U32(p, 0), 4) == G32(0x1017b8c8))
            SIM_CALL_1002c63a();
        Sim_100ac58e((int)U32(U32(p, 0), 0x154));
        U32(U32(p, 0), 0x154) = 0xffffffffu;
    }
}

// FUNCTION: HGSIM 0x100788fc
void __cdecl Sim_100788fc(void *obj, uint32_t a, uint32_t b)
{
    uint32_t e = U32(obj, 0xec);
    int half;

    SIM_CALL_1008d4f1(e, a);
    half = (int)S16(obj, 0x100) >> 1;
    SIM_CALL_100652e8(half, 2, b, e);
}
