#include "w3r.h"
#include "../unsorted_1009/w2i.h"
#include "../util/access.h"

/* The seven objects, and where the value for each is saved by Sim_10077ecd. */
static const uint32_t kObjects[7] = {0x109e8928u, 0x109e8938u, 0x109e890cu, 0x109e88e8u, 0x109e894cu, 0x109e8950u, 0x109e8954u};
static const uint32_t kSaved[7] = {0x1096fc48u, 0x1096fc4cu, 0x1096fc44u, 0x1096fc40u, 0x1096fc50u, 0x1096fc54u, 0x1096fc58u};
static const int32_t kDefaults[7] = {9, 10, 8, 1, 0, 12, 13};

static void CallMethod(uint32_t objectAddr, uint32_t value)
{
    ((void(__cdecl *)(uint32_t, uint32_t))(uintptr_t)U32(G32(objectAddr), 0x11c))(G32(objectAddr), value);
}

// FUNCTION: HGSIM 0x10077ecd
int __cdecl Sim_10077ecd(void)
{
    int i;

    if (G32(0x10179d20) != 0) {
        for (i = 0; i < 7; i++)
            CallMethod(kObjects[i], G32(kSaved[i]));
        G32(0x101355b8) = G32(0x1096fbec);
        G32(0x10144fac) = G32(0x1096fc1c);
        G32(0x1015d1cc) = G32(0x1096fc20);
        Sim_1009b30f(GS32(0x1096fc20));
    }
    return GS32(0x10179d20);
}

// FUNCTION: HGSIM 0x10077fff
void __cdecl Sim_10077fff(void)
{
    int i;

    for (i = 0; i < 7; i++)
        CallMethod(kObjects[i], (uint32_t)kDefaults[i]);
    G32(0x101355b8) = 1;
    G32(0x10144fac) = 1;
    G32(0x1015d1cc) = 1;
    Sim_1009b30f(1);
}
