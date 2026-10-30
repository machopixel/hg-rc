#include "w2l.h"
#include "../util/access.h"
#include "../util/util.h"
#include "../view/w2l.h"

// FUNCTION: HGSIM 0x10009764
void __cdecl Sim_10009764(void *obj)
{
    if (obj == 0)
        return;
    Sim_1008a16d(obj);
    Sim_1008a14f(obj);
}

// FUNCTION: HGSIM 0x10009787
void __cdecl Sim_10009787(void *obj)
{
    uint32_t owner = Sim_100096ee(obj);

    if (owner != 0)
        Sim_100a9b4c((void *)(uintptr_t)owner, 0);
    if (G32(0x1017e678) == (uint32_t)(uintptr_t)obj)
        G32(0x1017e678) = 0;
    Sim_10009764(obj);
}
