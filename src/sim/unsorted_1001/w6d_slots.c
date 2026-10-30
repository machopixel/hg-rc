#include "w6d.h"
#include "../unsorted_1002/w5z.h"
#include "../unsorted_1005/w5b.h"
#include "../util/access.h"

// FUNCTION: HGSIM 0x10015afd
void __cdecl Sim_10015afd(int index, uint32_t flags)
{
    char text[0x54];
    uint32_t rec;

    if ((uint32_t)index != G32(0x1017b8c8)) {
        if (G32(0x1015ecd0) == 0)
            return;
        if (U32(PTRTAB(index), 0x14) == 3 && U32(PTRTAB(index), 0xc) == G32(0x1017b8c8))
            return;
    }
    rec = PTRTAB(index);
    if ((uint32_t)U16(rec, 0x1a8) != (flags | 0x200u) || S16(rec, 0x1a6) != 3) {
        W6D_SPRINTF(text, SIM_PTR(const char, 0x10131364), G32(0x1015dc78), U32(rec, 8), index, flags);
        Sim_1005f9aa(text);
    }
    U16(rec, 0x1aa) = (uint16_t)(flags | 0x200u);
    S16(rec, 0x1a6) = 3;
}

// FUNCTION: HGSIM 0x100178af
uint32_t __cdecl Sim_100178af(int a, int b)
{
    if (a == -1) {
        Sim_1005259e((int)G32(0x1013bb40), SIM_PTR(const char, 0x109b68e0u + (uint32_t)b * 0x74u));
        return Sim_100203b8((uint32_t)b);
    }
    return W6D_CALL_10016117((uint32_t)b, (uint16_t)a);
}
