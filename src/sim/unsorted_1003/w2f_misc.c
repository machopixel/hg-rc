#include "w2f.h"
#include "../util/access.h"
#include "../unsorted_1007/fns.h"

// FUNCTION: HGSIM 0x10035599
void __cdecl Sim_10035599(uint32_t a, uint32_t b)
{
    (void)a;
    G32(0x109e3d80) = b;
}

// FUNCTION: HGSIM 0x100386d0
void __cdecl Sim_100386d0(void)
{
}

// FUNCTION: HGSIM 0x10038a9c
void __cdecl Sim_10038a9c(void *obj)
{
    uint32_t rec = U32(obj, 0x84) + U32(obj, 0x5fb) * 0x128;

    U32(rec, 8) = 0xffffffffu;
    U32(rec, 4) = 0xffffffffu;
}

// FUNCTION: HGSIM 0x1003cf81
void __cdecl Sim_1003cf81(void)
{
    int32_t key;

    for (key = 1; key <= 8; key++)
        Sim_100751e4(key);
}
