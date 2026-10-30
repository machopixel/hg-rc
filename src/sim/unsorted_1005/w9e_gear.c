#include "w9e.h"
#include "../unsorted_1004/fns.h"
#include "../unsorted_1004/w2h.h"
#include "../unsorted_1004/w2k.h"
#include "../unsorted_1004/w2l.h"
#include "../util/access.h"

/* Dword +0 of entry `i` of the table at 0x10124ea8 (0x98 bytes each). */
#define SPEC_SIZE(i) S32(GBYTES(0x10124ea8), (uint32_t)(i) * 0x98)

static void W9e_Max(int *best, int value)
{
    if (value > *best)
        *best = value;
}

static void W9e_MaxFlag(int *best, int flag, uint32_t addr)
{
    if (flag != 0)
        W9e_Max(best, GS32(addr));
}

// FUNCTION: HGSIM 0x1005a1db
int __cdecl Sim_1005a1db(const uint8_t *gear)
{
    int best = 0;
    int count = S32(gear, 0xb0);
    int i, kind, sel;

    for (i = 0; i < count; i++) {
        const uint8_t *item = BYTES(U32(gear, 0xc8)) + i * 0x6c;
        int type = S32(item, 4);
        uint32_t word = U32(item, 0xc);
        const uint8_t *def = GBYTES(0x10111688) + (uint32_t)type * 0xaa;

        W9e_Max(&best, S32(def, 0));
        kind = S32(def, 4);
        if (kind == 0)
            sel = 0;
        else if (kind == 1)
            sel = 1;
        else if (kind == 3)
            sel = 2;
        else if (kind == 4)
            sel = 3;
        else if (kind == 2)
            sel = 4;
        else if (kind < 5 || kind > 10)
            sel = -1;
        else
            sel = 5;
        switch (sel) {
        case 0:
            W9e_Max(&best, SPEC_SIZE(Sim_1004bf79(word)));
            W9e_Max(&best, SPEC_SIZE(Sim_1004bf87((int)word)));
            W9e_Max(&best, SPEC_SIZE(Sim_1004bf98((int)word)));
            W9e_MaxFlag(&best, Sim_1004bfa9((int)word), 0x10126cf0);
            break;
        case 1:
            W9e_Max(&best, SPEC_SIZE(Sim_1004c30d((int)word)));
            break;
        case 2:
            W9e_Max(&best, SPEC_SIZE(Sim_1004c565(word)));
            W9e_Max(&best, SPEC_SIZE(Sim_1004c573((int)word)));
            W9e_Max(&best, SPEC_SIZE(Sim_1004c584((int)word)));
            W9e_Max(&best, SPEC_SIZE(Sim_1004c595((int)word)));
            W9e_Max(&best, SPEC_SIZE(Sim_1004c5a6((int)word)));
            break;
        case 3:
            W9e_Max(&best, SPEC_SIZE(Sim_1004c909(word)));
            W9e_Max(&best, SPEC_SIZE(Sim_1004c917((int)word)));
            W9e_Max(&best, SPEC_SIZE(Sim_1004c928((int)word)));
            W9e_Max(&best, SPEC_SIZE(Sim_1004c939((int)word)));
            W9e_Max(&best, SPEC_SIZE(Sim_1004c94a((int)word)));
            break;
        case 4:
            W9e_MaxFlag(&best, Sim_1004cd8d(word), 0x101262d8);
            W9e_MaxFlag(&best, Sim_1004cd9e(word), 0x10126370);
            W9e_MaxFlag(&best, Sim_1004cdaf(word), 0x10126408);
            W9e_MaxFlag(&best, Sim_1004cdc0(word), 0x101264a0);
            W9e_MaxFlag(&best, Sim_1004cdd1(word), 0x10126538);
            W9e_MaxFlag(&best, Sim_1004cde2(word), 0x101265d0);
            W9e_MaxFlag(&best, Sim_1004cdf3(word), 0x10126668);
            W9e_MaxFlag(&best, Sim_1004ce04(word), 0x10126700);
            W9e_MaxFlag(&best, Sim_1004ce17(word), 0x10126798);
            W9e_MaxFlag(&best, Sim_1004ce2a(word), 0x10126830);
            W9e_MaxFlag(&best, Sim_1004ce3d(word), 0x101268c8);
            break;
        case 5:
            W9e_Max(&best, SPEC_SIZE(Sim_1004d26d(word)));
            break;
        default:
            break;
        }
    }
    return best;
}
