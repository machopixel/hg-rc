#include "w9f.h"
#include "w2h.h"
#include "w5a.h"
#include "w5c.h"
#include "w5y.h"
#include "w8v.h"
#include "../util/access.h"
#include "../unsorted_100f/w8z.h"

// FUNCTION: HGSIM 0x100473e4
void __cdecl Screenshot_Save(void)
{
    uint32_t region[5];
    char name[16];
    int32_t number = GS32(0x1015f150);

    region[0] = (uint32_t)(uintptr_t)GBYTES(0x109e3e20);
    region[1] = 0;
    region[2] = 0;
    region[3] = G32(0x109e3e14);
    region[4] = G32(0x109e3de0);
    if (number >= 1000)
        return;
    if (G32(0x1017b940) != 0) {
        G32(0x1015f150) = (uint32_t)number + 1u;
        Crt_SPrintf(name, (const char *)GBYTES(0x1013b2d0), number);          /* "hg%04d.tga" */
        Screenshot_WriteTga(name, (int32_t)(G32(0x109e3e14) + 1u), (int32_t)(G32(0x109e3de0) + 1u),
                            (int32_t)(G32(0x109e3e24) + 1u), BYTES(G32(0x109e3e20)));
    } else {
        G32(0x1015f150) = (uint32_t)number + 1u;
        Crt_SPrintf(name, (const char *)GBYTES(0x1013b2dc), number);          /* "hg%04d.gif" */
        Sim_1004ec60((int)(uintptr_t)name);
        Sim_1004ecb8();
        Sim_1004ed37((const uint8_t *)region);
        Sim_1004edb6();
    }
}

/* The byte table at 0x1004b9ef folded with the jump table at 0x1004b95b: code -> result. */
static const uint8_t W9f_CodeMap[0x5f] = {
    0, 2, 2, 1, 0, 4, 4, 4, 4, 0, 0, 0, 0, 3, 3, 3, 3, 0, 0, 0, 0, 0, 6, 6, 6, 0, 0, 5, 5, 5, 0, 0,
    7, 14, 16, 13, 15, 0, 11, 9, 0, 12, 10, 0, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 6, 5,
    5, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 6, 5, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4,
};

// FUNCTION: HGSIM 0x1004b8b0
uint32_t __cdecl Sim_1004b8b0(uint32_t code)
{
    if (code > 0x5e)
        return 0;
    return W9f_CodeMap[code];
}
