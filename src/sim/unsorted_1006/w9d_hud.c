#include <string.h>
#include "w9d.h"
#include "w7n.h"
#include "w8w.h"
#include "../render/w2n.h"
#include "../unsorted_1000/w5a.h"
#include "../unsorted_1004/fns.h"
#include "../unsorted_100b/w6g.h"
#include "../unsorted_100c/w2f.h"
#include "../unsorted_100c/w7q.h"
#include "../unsorted_100d/w8z.h"
#include "../unsorted_100f/w2e.h"
#include "../util/access.h"

static float W9d_Float(const void *p, uint32_t off)
{
    float f;

    memcpy(&f, BYTES(p) + off, 4);
    return f;
}

// FUNCTION: HGSIM 0x10063037
void __cdecl Sim_10063037(void *holder)
{
    uint32_t obj = U32(holder, 0);
    uint32_t target = U32(obj, 0x118);
    uint32_t index;
    int row;
    char text[16];
    float x, y, z;

    if (target == 0 || (target & 0x1000) != 0)
        return;
    index = target & 0xff;
    switch (target & 0xf00) {
    case 0x100: {
        int32_t counter = GS32(0x10176954);
        uint32_t str = U32(GBYTES(0x109e857c), (counter % 7) * 4);

        G32(0x10176954) = (uint32_t)counter + 1;
        Str_Copy(text, (const char *)(uintptr_t)str);
        row = S16(GBYTES(0x109a8664), index * 0x54) == 0x20 ? 7 : 5;
        break;
    }
    case 0x200: {
        int kind = Sim_10052be7((int)index);

        W9D_CALL_100648a0(PTRTAB(index), kind);
        return;
    }
    case 0x400: {
        int kind = Sim_10052c02((int)index);
        uint32_t value = Sim_100cba68();

        W9D_CALL_10064d05(index, value, kind);
        return;
    }
    default:
        return;
    }
    x = W9d_Float(BYTES(obj), 0x104);
    y = W9d_Float(BYTES(obj), 0x108);
    z = W9d_Float(BYTES(obj), 0x10c);
    if (Sim_100d2669(&x, &y, &z) == 0)
        return;
    if (G32(0x1017b940) != 0)
        Sim_10065216((uint32_t)Render_Ftol((double)x), (uint32_t)Render_Ftol((double)y), text,
                     (uint32_t)(uintptr_t)(GBYTES(0x10176960) + row * 0x200));
    else
        Sim_10065216((uint32_t)Render_Ftol((double)x), (uint32_t)Render_Ftol((double)y), text,
                     (uint32_t)(uintptr_t)(GBYTES(0x10178b60) + row * 0x100));
}

// FUNCTION: HGSIM 0x10065384
void __cdecl Sim_10065384(void)
{
    char *fontname = (char *)(uintptr_t)G32(0x109b9900);
    uint32_t font = W9D_CALL_100ba7d5(fontname, G32(0x1014cec4), 0);
    int width, height;
    int32_t x, y;
    double scale;

    if (font == 0)
        return;
    U8(GBYTES(0x109b992e), 0) = 7;
    width = Sim_100ccb0d(SIM_PTR(char, 0x101658d8), (void *)(uintptr_t)font);
    height = W9D_CALL_100ecce0(font);
    x = (int32_t)(G32(0x109e3e10) - (uint32_t)height * 3);
    memcpy(&scale, GBYTES(0x1012b5e0), 8);
    y = Render_Ftol((double)GS32(0x109e3e18) - (double)width * scale);
    if (G32(0x101668e8) != 0)
        W9D_CALL_100f2f01(0x109e3e00, y, x, font, SIM_PTR(char, 0x101658d8), 0x109b9920);
    else if (G32(0x1017b940) != 0)
        W9D_CALL_100f1e10(0x109e3e00, y, x, font, SIM_PTR(char, 0x101658d8), 0x109b9920);
    else
        W9D_CALL_100ecea6(0x109e3e00, y, x, font, SIM_PTR(char, 0x101658d8), 0x109b9920);
    U8(GBYTES(0x109b992e), 0) = 9;
    Sim_100bb2b0((const uint32_t *)fontname, (const char *)(uintptr_t)G32(0x1014cec4));
}

// FUNCTION: HGSIM 0x1006561a
void __cdecl Sim_1006561a(void)
{
    if (G32(0x101658c0) == 0)
        return;
    G32(0x101658c4) = G32(0x101658c4) + G32(0x1015dc80);
    if (GS32(0x101658c4) >= GS32(0x101658c8)) {
        G32(0x101658cc) = G32(0x101658cc) - 1;
        if (G32(0x101658cc) != 0) {
            if (G32(0x101658d4) != 0)
                Sim_1000ebfb(SIM_PTR(char, 0x10145048), 0xfffffc18u, 0, 5, 10);
            G32(0x101658d4) = G32(0x101658d4) ^ 1;
            G32(0x101658c4) = 0;
        } else {
            G32(0x101658c0) = 0;
            G32(0x101658cc) = 0;
            G32(0x101658d4) = 0;
        }
    }
    if (G32(0x101658d4) != 0)
        Sim_10065384();
}

// FUNCTION: HGSIM 0x100690d4
int __cdecl Sim_100690d4(uint8_t **a, uint8_t **b, float *x, float *y, float *z)
{
    int result = Sim_10068e77(a, b, x, y, z);

    if (result == 0)
        result = Sim_10068e77(b, a, x, y, z);
    return result;
}
