#include "w3r.h"
#include "../util/w3r.h"

// FUNCTION: HGSIM 0x10037c6c
void __cdecl Sim_10037c6c(void)
{
    if (G32(0x1015e68c) != 0)
        ((void(__cdecl *)(void))(uintptr_t)U32(G32(0x1015e68c), 0x14))();
    if (G32(0x1015e688) != 0)
        ((void(__cdecl *)(void))(uintptr_t)U32(G32(0x1015e688), 0x10))();
    G32(0x1013a2c8) = 1;
}

// FUNCTION: HGSIM 0x1003c970
void __cdecl Sim_1003c970(void)
{
    int32_t q;

    G32(0x1015ebb0) = 1;
    q = GS32(0x1013abc8) / 0x18;
    GS32(0x1015eba0) = (int32_t)((uint32_t)q * 5u);
    GS32(0x1015eba4) = (int32_t)((uint32_t)q * 7u);
    GS32(0x1015eba8) = (int32_t)((uint32_t)q * 0x11u);
    GS32(0x1015ebac) = (int32_t)((uint32_t)q * 0x13u);
    if (GS32(0x1013abb8) != -1)
        GS32(0x1013abd0) = GS32(0x1015eba0u + (uint32_t)GS32(0x1013abb8) * 4u);
    GS32(0x1013abd4) = GS32(0x1013abd0);
    GS32(0x1013abb8) = -1;
    GF32(0x1013abc4) = (float)((double)GF32(0x1013abc0) / (double)GF32(0x101114f8));
}

// FUNCTION: HGSIM 0x1003cc73
void __cdecl Sim_1003cc73(void)
{
    int32_t i;

    for (i = 0; i < GS32(0x1015dc64); i++) {
        uint32_t data = U32(PTRTAB(i), 0x28);

        F32(data, 0x4ec) = (float)((double)F32(data, 0x4ec) * (double)GF32(0x101114fc));
    }
}
