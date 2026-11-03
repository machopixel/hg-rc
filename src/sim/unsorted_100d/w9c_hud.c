#include "w9c.h"
#include "../unsorted_1008/w2h.h"
#include "../unsorted_1008/w7o.h"
#include "../unsorted_100b/w6g.h"
#include "../unsorted_100f/w8z.h"
#include "../util/access.h"

typedef uint32_t(__cdecl *W9cDResourceFn)(const char *name, const char *ext, void *out);
typedef int(__cdecl *W9cDShapeFn)(uint32_t pane, uint32_t shape, int32_t a, int32_t x, int32_t y);
typedef int(__cdecl *W9cDTextFn)(uint32_t pane, int32_t x, int32_t y, uint32_t font, const char *text, void *pal);

#define W9CD_CALL_100ba7d5 SIM_FN(W9cDResourceFn, 0x100ba7d5)
#define W9CD_CALL_100f0205 SIM_FN(W9cDShapeFn, 0x100f0205)
#define W9CD_CALL_100e91dc SIM_FN(W9cDShapeFn, 0x100e91dc)
#define W9CD_CALL_100f2f01 SIM_FN(W9cDTextFn, 0x100f2f01)
#define W9CD_CALL_100f1e10 SIM_FN(W9cDTextFn, 0x100f1e10)
#define W9CD_CALL_100ecea6 SIM_FN(W9cDTextFn, 0x100ecea6)

#define W9CD_EXT_SHP ((const char *)(uintptr_t)G32(0x1014cec0))
#define W9CD_EXT_FONT ((const char *)(uintptr_t)G32(0x1014cec4))

static void W9cD_Shape(uint32_t pane, uint32_t shape, int32_t x, int32_t y)
{
    W9CD_CALL_100f0205(pane, shape, 0, x, y);
}

static void W9cD_Text(uint32_t pane, int32_t x, int32_t y, uint32_t font, const char *text)
{
    if (G32(0x101668e8) != 0)
        W9CD_CALL_100f2f01(pane, x, y, font, text, GBYTES(0x109b9920));
    else
        W9CD_CALL_100f1e10(pane, x, y, font, text, GBYTES(0x109b9920));
}

// FUNCTION: HGSIM 0x100d3d10
void __cdecl Sim_100d3d10(void *panel)
{
    char text[0x40];
    uint8_t *block;
    uint8_t *weapon;
    const char *kindName;
    uint32_t colour = 0;
    uint32_t font;
    uint32_t shape;

    if (S32(panel, 0xc) < 0 || G32(0x101355b8) == 0)
        return;
    block = BYTES(U32(PTRTAB(GS32(0x1017b8c8)), 0x28));
    weapon = BYTES(U32(block, 0x84)) + (uint32_t)S32(panel, 0xc) * 0x128u;
    if (S32(weapon, 4) < 0)
        return;
    switch (U32(weapon, 8) + 1) {
    case 0:
        colour = 0xf0;
        break;
    case 1:
        colour = 0x14;
        break;
    case 2:
        switch (U32(weapon, 0xc)) {
        case 0:
            colour = 0xf;
            break;
        case 1:
            colour = 0x10;
            break;
        case 2:
            colour = 7;
            break;
        case 3:
            colour = 8;
            break;
        case 4:
            colour = 0x17;
            break;
        default:
            break;
        }
        break;
    case 3:
        colour = 4;
        break;
    default:
        colour = 0xf;
        break;
    }
    font = W9CD_CALL_100ba7d5((const char *)(uintptr_t)G32(0x109b98c0), W9CD_EXT_FONT, 0);
    if (font == 0)
        return;
    if (S32(panel, 0xc) == S32(block, 0x4fc))
        Sim_1008d405((const void *)(uintptr_t)U32(panel, 0xec));
    kindName = (const char *)GBYTES(0x10145508) + (uint32_t)S32(weapon, 4) * 0xd6u;
    shape = W9CD_CALL_100ba7d5((const char *)(uintptr_t)G32(0x109e84b8), W9CD_EXT_SHP, 0);
    if (shape != 0) {
        W9cD_Shape(U32(panel, 0xec), shape, 0, 0);
        Sim_100bb2b0((const uint32_t *)(uintptr_t)G32(0x109e84b8), W9CD_EXT_SHP);
    }
    kindName += S32(weapon, 8) == -1 ? 0xa2 : 0x92;
    shape = W9CD_CALL_100ba7d5(kindName, W9CD_EXT_SHP, 0);
    if (shape != 0) {
        if (G32(0x101c64f0) != 0) {
            uint32_t at = U32(panel, 0xf0);

            W9cD_Shape(U32(panel, 0xec), shape, S32(at, 0), S32(at, 4));
        }
        Sim_100bb2b0((const uint32_t *)kindName, W9CD_EXT_SHP);
    }
    *GBYTES(0x109b992e) = (uint8_t)colour;
    Crt_SPrintf(text, SIM_PTR(const char, 0x10152434), (const char *)panel + 0x10);        /* "%s" */
    W9cD_Text(U32(panel, 0xec), S32(panel, 0xd4), S32(panel, 0xd8), font, text);
    if (S32(weapon, 0x20) > 0) {
        Crt_SPrintf(text, SIM_PTR(const char, 0x10152438), S32(weapon, 0x20));             /* "%d" */
        W9cD_Text(U32(panel, 0xec), S32(panel, 0xdc), S32(panel, 0xe0), font, text);
    }
    if (S32(panel, 0xc) == S32(block, 0x4fc))
        Sim_1008d4f1((int32_t *)(uintptr_t)U32(panel, 0xec), (int32_t)colour);
    *GBYTES(0x109b992e) = 9;
    Sim_100bb2b0((const uint32_t *)(uintptr_t)G32(0x109b98c0), W9CD_EXT_FONT);
}
