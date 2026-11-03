#include "w12.h"
#include "../simmem.h"
#include "../util/access.h"
#include "../util/w3o.h"
#include "../render/w2n.h"
#include "../unsorted_1004/fns.h"
#include "../unsorted_1006/w8w.h"
#include "../unsorted_100b/w6g.h"

/* The resource loader, sprintf and the HUD 2D library of the DLL (left original). */
#define W12C_LOAD SIM_FN(uint32_t(__cdecl *)(uint32_t, uint32_t, uint32_t), 0x100ba7d5)
#define W12C_SPRINTF SIM_FN(int(__cdecl *)(char *, const char *, ...), 0x100fc780)
#define W12C_BLIT_F0205 SIM_FN(void(__cdecl *)(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t), 0x100f0205)
#define W12C_BLIT_E91DC SIM_FN(void(__cdecl *)(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t), 0x100e91dc)
typedef void(__cdecl *W12cTextFn)(uint32_t, uint32_t, uint32_t, uint32_t, char *, uint32_t);

static void W12C_Release(uint32_t name, uint32_t set)
{
    Sim_100bb2b0((const uint32_t *)(uintptr_t)name, (const char *)(uintptr_t)set);
}

static void W12C_Rotated(uint32_t canvas, uint32_t shape, int32_t angle)
{
    uint32_t at = U32(G32(0x109e8940), 0xf0);

    Sim_1006419e(canvas, shape, 0, S32(at, 0), S32(at, 4), (uint8_t *)(uintptr_t)G32(0x1015dc2c), (uint32_t)angle,
                 0x10000, 0x10000, 0);
}

// FUNCTION: HGSIM 0x10064364
void __cdecl Sim_10064364(float yaw, float pitch)
{
    uint32_t canvas = U32(G32(0x109e8940), 0xec);
    float heading = 0.0f; /* start value (DECISIONS.md): the heading text without the dial */
    uint32_t shape, font, self, target;
    char text[8];

    shape = W12C_LOAD(G32(0x109e8484), G32(0x1014cec0), 0);
    if (shape != 0) {
        heading = (float)(Render_Ftol((double)GFL(0x1012b5d8) - (double)yaw) % 360);
        W12C_Rotated(canvas, shape, Render_Ftol((double)heading * (double)GFL(0x1012b5dc)));
        W12C_Release(G32(0x109e8484), G32(0x1014cec0));
    }
    shape = W12C_LOAD(G32(0x109e8480), G32(0x1014cec0), 0);
    if (shape != 0) {
        W12C_BLIT_F0205(canvas, shape, 0, 0, 0);
        W12C_Release(G32(0x109e8480), G32(0x1014cec0));
    }
    shape = W12C_LOAD(G32(0x109e8488), G32(0x1014cec0), 0);
    if (shape != 0) {
        uint32_t unit = PTRTAB(G32(0x1017b8c8));

        if (U32(unit, 0x118) != 0xffffffffu && G32(0x109a8640 + (U32(unit, 0x118) & 0xff) * 0x54) != 0) {
            int32_t bearing = Render_Ftol((double)GFL(0x1012b5d8) - ((double)yaw - (double)GFL(0x1015e4d4)));

            W12C_Rotated(canvas, shape, bearing * 10);
        }
        W12C_Release(G32(0x109e8488), G32(0x1014cec0));
    }
    self = PTRTAB(G32(0x1017b8c8));
    target = U32(self, 0xf0) & 0xff;
    if (U32(self, 0xf0) != 0xffffffffu && (U32(self, 0xf0) & 0xf00) == 0x200 && PTRTAB(target) != 0 &&
        U32(PTRTAB(target), 0x28) != 0 && U32(U32(PTRTAB(target), 0x28), 0x4f0) == 2) {
        uint32_t name = Sim_10052be7((int)target) == 1 ? G32(0x109e848c) : G32(0x109e85c8);

        shape = W12C_LOAD(name, G32(0x1014cec0), 0);
        W12C_Rotated(canvas, shape, Render_Ftol((double)pitch * (double)GFL(0x1012b5dc)));
        W12C_Release(name, G32(0x1014cec0));
    }
    font = W12C_LOAD(G32(0x109b9900), G32(0x1014cec4), 0);
    if (font != 0) {
        uint32_t x = U32(G32(0x109e8940), 0xb0), y = U32(G32(0x109e8940), 0xac);
        W12cTextFn draw;

        U8(0x109b9900, 0x2e) = 0xc;
        W12C_SPRINTF(text, (const char *)GBYTES(0x10145044), Render_Ftol((double)heading));
        if (G32(0x101668e8) != 0)
            draw = SIM_FN(W12cTextFn, 0x100f2f01);
        else
            draw = SIM_FN(W12cTextFn, 0x100f1e10);
        draw(canvas, y, x, font, text, 0x109b9920);
        U8(0x109b9900, 0x2e) = 9;
        W12C_Release(G32(0x109b9900), G32(0x1014cec4));
    }
}