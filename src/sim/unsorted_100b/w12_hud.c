#include "w12.h"
#include "../simmem.h"
#include "../util/access.h"
#include "../util/w3o.h"
#include "../render/w2n.h"
#include "w6g.h"

/* The resource loader and the HUD 2D library of the DLL (left original). */
#define W12_LOAD SIM_FN(uint32_t(__cdecl *)(uint32_t, uint32_t, uint32_t), 0x100ba7d5)
#define W12_DRAW_F0205 SIM_FN(void(__cdecl *)(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t), 0x100f0205)
#define W12_DRAW_E91DC SIM_FN(void(__cdecl *)(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t), 0x100e91dc)
#define W12_DRAW_E96C6 SIM_FN(void(__cdecl *)(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t), 0x100e96c6)
#define W12_COLOURS_F01AD SIM_FN(void(__cdecl *)(uint32_t), 0x100f01ad)
#define W12_COLOURS_E96A7 SIM_FN(void(__cdecl *)(uint32_t), 0x100e96a7)

// FUNCTION: HGSIM 0x100b66a0
void __cdecl Sim_100b66a0(void)
{
    int32_t level = 0, style, i;
    int32_t divisor = 1; /* start value (DECISIONS.md): non-zero, as the original saw in every call */
    uint32_t rec = U32(PTRTAB(G32(0x1017b8c8)), 0x28);
    uint32_t shape = W12_LOAD(G32(0x109e8498), G32(0x1014cec0), 0);

    if (shape != 0) {
        W12_DRAW_F0205(0x109e8674, shape, 0, 0, 0);
        Sim_100bb2b0((const uint32_t *)(uintptr_t)G32(0x109e8498), (const char *)(uintptr_t)G32(0x1014cec0));
    }
    for (i = 0; i < 7; i++) {
        uint32_t seg = U32(rec, 0x88) + (G32(0x101508a0 + (uint32_t)i * 8) - 1) * 0x3c;
        uint32_t name;

        if (divisor != 0) {
            divisor = S32(seg, 0x38);
            level = 15 - Render_Ftol(((double)FL(seg, 8) * (double)GFL(0x1012bae0) +
                                      (double)FL(seg, 0) * (double)GFL(0x1012bae0)) *
                                     (double)GFL(0x1012bae4) / (double)divisor);
        }
        if (level < 1)
            level = 0;
        else if (level > 15)
            level = 15;
        if (level == 15 && (double)FL(seg, 8) > *SIM_PTR(const double, 0x1012bae8))
            level = 14;
        if (level == 15)
            style = 8;
        else if (level == 0)
            style = 5;
        else if (level < 8)
            style = 7;
        else
            style = 4;
        name = G32(0x109e8480 + G32(0x101508a4 + (uint32_t)i * 8) * 4);
        shape = W12_LOAD(name, G32(0x1014cec0), 0);
        if (shape == 0)
            continue;
        if (G32(0x1017692c) != 0) {
            uint32_t at = U32(G32(0x109e88e4), 0xf0);

            W12_COLOURS_F01AD(0x10176960 + ((uint32_t)style << 9));
            W12_DRAW_F0205(0x109e8674, shape, 0, U32(at, 0), U32(at, 4));
            W12_COLOURS_F01AD(0x101659e0);
        }
        Sim_100bb2b0((const uint32_t *)(uintptr_t)G32(0x109e8480 + G32(0x101508a4 + (uint32_t)i * 8) * 4),
                     (const char *)(uintptr_t)G32(0x1014cec0));
    }
}