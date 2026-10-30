#include "w3p.h"
#include "../util/access.h"
#include "../util/w3p.h"

typedef void(__cdecl *Fn0)(void);
typedef void(__cdecl *Fn3)(uint32_t, uint32_t, uint32_t);

// FUNCTION: HGSIM 0x10037e8f
void __cdecl Sim_10037e8f(void)
{
    uint8_t *best = 0;
    int i;

    for (i = 0; i < 7; i++) {
        uint8_t *p = BYTES(SIM_PTR(u32a, 0x1013a2a8)[i]);

        if (U32(p, 8) != 0 && U32(p, 0xc) != 0 && (best == 0 || U32(p, 0xc) < U32(best, 0xc)))
            best = p;
    }
    if (G32(0x1015e68c) != (uint32_t)(uintptr_t)best) {
        (*(Fn0 *)(BYTES(G32(0x1015e68c)) + 0x14))();
        G32(0x1015e68c) = (uint32_t)(uintptr_t)best;
        (*(Fn3 *)(BYTES(G32(0x1015e68c)) + 0x10))(G32(0x1015e698), G32(0x109e3d14), G32(0x109e3d18));
    }
    G32(0x1015e690) = G32(0x1015e68c);
    G32(0x10179d68) = 1;
    SIM_FN(void(__cdecl *)(const void *, uint32_t, uint32_t), 0x10083495)(
        GBYTES(0x1013a340), U32(G32(0x1015e68c), 0), U32(G32(0x1015e68c), 0xc));
}

// FUNCTION: HGSIM 0x1003ccc4
void __cdecl Sim_1003ccc4(void)
{
    int i;

    for (i = 0; i < GS32(0x1015dc64); i++) {
        uint8_t *block = BYTES(U32(PTRTAB(i), 0x28));

        F32(block, 0x4ec) = (float)((double)F32(block, 0x4ec) / (double)GF32(0x101114fc));
    }
}
