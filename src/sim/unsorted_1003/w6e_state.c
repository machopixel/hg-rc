#include "w6e.h"
#include "../unsorted_100d/w5x.h"
#include "../unsorted_1002/w5a.h"
#include "../unsorted_1006/w5b.h"
#include "../unsorted_1007/w5a.h"
#include "../util/access.h"

// FUNCTION: HGSIM 0x1003cbb3
void __cdecl Sim_1003cbb3(uint32_t unused, int mode)
{
    (void)unused;
    if (GS32(0x109b9e08) == mode)
        return;
    if (mode == 1) {
        if (S16(G32(0x109e88e0), 6) < 1) {
            G32(0x109b9e08) = 1;
            G32(0x1015ebb0) = 0;
            Sim_100d3830(0xc, 0xb5);
            G32(0x10143fdc) = 0x4b;
            G32(0x10143fe0) = 0xff;
            G32(0x10143fe4) = 0x4b;
            Sim_100203cf(0, 1);
        }
    } else if (mode == 0) {
        G32(0x1015ebb0) = 1;
        G32(0x1013abb8) = 0xffffffffu;
        G32(0x109b9df8) = 0;
        G32(0x10143fdc) = 0xff;
        G32(0x10143fe0) = 0xff;
        G32(0x10143fe4) = 0xff;
    }
}

// FUNCTION: HGSIM 0x1003ce98
void __cdecl Sim_1003ce98(uint32_t a, uint32_t b, int value, uint32_t d, uint32_t e, void *obj)
{
    int matchGlobal, matchObj;

    (void)a;
    (void)b;
    (void)d;
    (void)e;
    if (obj == 0)
        return;
    matchGlobal = value == (int)(G32(0x1015dc18) - 0x31u);
    matchObj = value == S32(obj, 0x10);
    if ((matchObj && G32(0x1015dc18) == 0xd) || matchGlobal) {
        G32(0x1015ebf8) = 1;
        U8(G32(0x1017b8c4), 1) = 0;
        Sim_1006c9a0((void *)(uintptr_t)U32(PTRTAB(G32(0x1017b8c8)), 0x28), 0);
        Sim_10074c8f();
    }
}
