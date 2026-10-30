#include "w3s.h"
#include "w2k.h"
#include "../render/w3s_fp.h"
#include "../core/core.h"

#define KEY S16(GBYTES(0x1015fd4c), 0)
#define CODE G32(0x1015dc18)

// FUNCTION: HGSIM 0x10075371
void __cdecl Sim_10075371(void)
{
    uint32_t node;
    uint32_t flags;

    CODE = 0;
    node = Sim_10076689();
    if (node == 0)
        return;
    flags = U32(node, 4);
    if (KEY != 0) {
        int key = KEY;

        if (key == 0x1b) {
            CODE = (uint32_t)KEY;
        } else if (key >= 0x30 && key <= 0x39) {
            CODE = (uint32_t)KEY;
        } else if ((flags & 1) != 0) {
            int ok = 0;

            if (key <= 0xd)
                ok = key == 0xd || key == 9;
            else if (key <= 0xc9)
                ok = key >= 0xc6 || key == 0x20;
            else
                ok = key == 0x209;
            if (ok)
                CODE = (uint32_t)KEY;
        }
        if (CODE == (uint32_t)(int)KEY)
            KEY = 0;
    } else if ((flags & 1) != 0) {
        if (*SIM_PTR(int8_t, 0x1015fde6) != 0)
            CODE = 0xd;
        else if (*SIM_PTR(int8_t, 0x1015fde7) != 0)
            CODE = 0x1b;
        if (CODE == 0 && Timer_Elapsed(G32(0x10149948)) > 0x5a) {
            float v = GF32(0x1015fdd8);

            if (!(v >= GF32(0x1012b7f0)))
                CODE = 0xc6;
            else if (v > GF32(0x1012b7f4))
                CODE = 0xc7;
        }
        if (CODE == 0 && Timer_Elapsed(G32(0x10149948)) > 0x2d) {
            float v = GF32(0x1015fddc);

            if (!(v >= GF32(0x1012b7f0)))
                CODE = 0xc9;
            else if (v > GF32(0x1012b7f4))
                CODE = 0x20;
        }
    }
    if (CODE != 0 && (flags & 1) != 0) {
        Timer_Reset(G32(0x10149948));
        *SIM_PTR(uint8_t, 0x1015fde2) = 1;
        *SIM_PTR(uint8_t, 0x1015fde5) = 1;
    }
}
