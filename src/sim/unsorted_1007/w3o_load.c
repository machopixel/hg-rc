#include "w3o.h"
#include "w2j.h"
#include "../core/core.h"
#include "../unsorted_1005/w2j.h"
#include "../util/w3o.h"
#include "../view/view.h"

/* The cache loader (thunk 0x10003413, not converted): (name, argument, 0) -> data or NULL. */
#define LOAD(name, arg) SIM_FN(uint32_t(__cdecl *)(uint32_t, uint32_t, int), 0x10003413)((name), (arg), 0)

// FUNCTION: HGSIM 0x10074fce
void __cdecl Sim_10074fce(void *obj)
{
    if (U32(obj, 0x10) != 0)
        U32(obj, 0x14) = LOAD(U32(obj, 0x10), G32(0x1014cec0));
    else
        U32(obj, 0x14) = 0;
    if (U32(obj, 0x1c) != 0)
        U32(obj, 0x20) = LOAD(U32(obj, 0x1c), G32(0x1014cec0));
    else
        U32(obj, 0x20) = 0;
    U32(obj, 0x44) = G32(0x109b98c0);
    U32(obj, 0x48) = LOAD(U32(obj, 0x44), G32(0x1014cec4));
}

// FUNCTION: HGSIM 0x10075063
int __cdecl Sim_10075063(int *p)
{
    void *obj = 0;

    if (GS32(0x10149948) == -1)
        G32(0x10149948) = (uint32_t)(int32_t)(int16_t)Timer_Alloc(0x100);
    Timer_Reset(G32(0x10149948));
    if (p[0] >= 1 && p[0] <= 0xd) {
        p[3] = (int)G32(0x10149958u + (uint32_t)p[0] * 4);
        obj = (void *)(uintptr_t)p[3];
    }
    if (obj == 0)
        return 0;
    Sim_10076667(obj);
    Sim_10076624(obj, (int32_t)U32(obj, 0x80));
    G32(0x1015dbcc) = G32(0x1015dbcc) + 1;
    if (U32(obj, 4) & 1)
        Sim_1005570e();
    return 1;
}
