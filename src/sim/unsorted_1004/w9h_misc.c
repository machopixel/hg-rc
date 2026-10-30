#include "w9h.h"
#include "fns.h"
#include "../util/access.h"
#include "../unsorted_1001/w3s.h"
#include "../unsorted_1001/w5z.h"
#include "../unsorted_1003/fns.h"

// FUNCTION: HGSIM 0x10042a11
void __cdecl Sim_10042a11(void)
{
    int32_t i, idx;
    uint32_t rec;

    for (i = 0; i < GS32(0x1015dc64); i++) {
        rec = PTRTAB(i);
        if (S32(rec, 0x14) == 3)
            U32(rec, 0xc) = G32(0x1017b8c8);
    }
    Sim_10010f70();
    for (i = 0; i < GS32(0x1015dc64); i++) {
        rec = PTRTAB(i);
        idx = Sim_10052aaf(S32(rec, 8));
        if (idx == -1) {
            idx = Sim_10012843(S32(rec, 8));
            if (idx == -1)
                idx = GS32(0x1017b8c8);
        }
        if (S32(PTRTAB(idx), 0x14) != 0 && S32(PTRTAB(idx), 0x14) != 1)
            idx = (int32_t)Sim_1003db56((int16_t)U16(GBYTES(0x1015ecd8), 0));
        if (S32(rec, 0x14) == 3)
            U32(rec, 0xc) = (uint32_t)idx;
    }
}

// FUNCTION: HGSIM 0x1004d344
int32_t __cdecl Sim_1004d344(int index)
{
    return S32(GBYTES(0x101116a6), (uint32_t)index * 0xaa);
}
