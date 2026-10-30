#include "w2i.h"
#include "../util/access.h"

// FUNCTION: HGSIM 0x1009b30f
void __cdecl Sim_1009b30f(int v)
{
    if (GS32(0x1015D1AC) == v)
        return;
    if (GS32(0x1015D1AC) <= 2 && v <= 2) {
        GS32(0x1015D1CC) = v;
        return;
    }
    if (GS32(0x1015D1AC) <= 2) {
        GS32(0x1015D1CC) = 3;
        return;
    }
    GS32(0x1015D1CC) = 5;
    GS32(0x1015D098) = v;
}

// FUNCTION: HGSIM 0x1009c9e5
uint32_t __cdecl Sim_1009c9e5(void)
{
    uint32_t result = G32(0x101CFCE0u + G32(0x1017CD44) * 4u);

    G32(0x1017CD44) = G32(0x1017CD44) + 1u;
    GS32(0x1017CD44) = GS32(0x1017CD44) % 0x7f;
    return result;
}
