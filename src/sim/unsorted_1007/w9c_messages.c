#include "w9c.h"
#include "../core/w2a.h"
#include "../util/access.h"

typedef uint32_t(__cdecl *W9c7ResourceFn)(const char *name, const char *ext, void *out);
typedef int(__cdecl *W9c7ShapeFn)(uint32_t pane, uint32_t shape, int32_t a, int32_t x, int32_t y);
typedef int(__cdecl *W9c7TextFn)(uint32_t pane, int32_t x, int32_t y, uint32_t font, const char *text, void *pal);

#define W9C7_CALL_100ba7d5 SIM_FN(W9c7ResourceFn, 0x100ba7d5)
#define W9C7_CALL_100f0205 SIM_FN(W9c7ShapeFn, 0x100f0205)
#define W9C7_CALL_100e91dc SIM_FN(W9c7ShapeFn, 0x100e91dc)
#define W9C7_CALL_100f2f01 SIM_FN(W9c7TextFn, 0x100f2f01)
#define W9C7_CALL_100f1e10 SIM_FN(W9c7TextFn, 0x100f1e10)
#define W9C7_CALL_100ecea6 SIM_FN(W9c7TextFn, 0x100ecea6)

// FUNCTION: HGSIM 0x1007e0ff
void __cdecl Sim_1007e0ff(void)
{
    int i;

    for (i = 0; i < 2; i++) {
        uint8_t *m = GBYTES(0x10149d28) + i * 0x3c;
        uint32_t font, shape, pane, text, x, y;

        if (U32(m, 0xc) == 0)
            continue;
        if (Sim_1002ab9d() >= S32(m, 0x34)) {
            U32(m, 0xc) = 0;
            continue;
        }
        font = W9C7_CALL_100ba7d5((const char *)m + 0x14, (const char *)(uintptr_t)G32(0x1014cec4), 0);
        if (font == 0)
            continue;
        shape = W9C7_CALL_100ba7d5((const char *)m + 0x24, (const char *)(uintptr_t)G32(0x1014cec0), 0);
        if (shape == 0)
            continue;
        *GBYTES(0x109b992e) = 0xfe;
        W9C7_CALL_100f0205(U32(m, 0x38), shape, 0, 0, 0);
        text = U32(m, 0);
        y = U32(m, 8);
        x = U32(m, 4);
        pane = U32(m, 0x38);
        if (G32(0x101668e8) != 0)
            W9C7_CALL_100f2f01(pane, (int32_t)x, (int32_t)y, font, (const char *)(uintptr_t)text, GBYTES(0x109b9920));
        else
            W9C7_CALL_100f1e10(pane, (int32_t)x, (int32_t)y, font, (const char *)(uintptr_t)text, GBYTES(0x109b9920));
        *GBYTES(0x109b992e) = 9;
    }
}
