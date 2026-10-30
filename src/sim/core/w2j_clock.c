#include "w2j.h"
#include "../util/access.h"

// FUNCTION: HGSIM 0x1002a983
void __cdecl Sim_1002a983(void)
{
    if (G32(0x1015dca0) == 0)
        return;
    if (G32(0x1015dc84) == 0) {
        G32(0x1015dc78) = Timer_Elapsed(G32(0x101354d0));
        if (GS32(0x1015dc94) > 0) {
            while ((int32_t)(G32(0x1015dc78) - G32(0x1015dc98)) < GS32(0x1015dc94))
                G32(0x1015dc78) = Timer_Elapsed(G32(0x101354d0));
        }
        if (G32(0x1015dc88) != 0) {
            G32(0x1015dc9c) = G32(0x1015dc84);
            G32(0x1015dc84) = 3;
            Timer_Pause(0x80, 1);
        }
    } else if (G32(0x1015dc84) == 3) {
        G32(0x1015dc78) += 0xc;
        if (G32(0x1015dc88) == 0) {
            G32(0x1015dc84) = G32(0x1015dc9c);
            Timer_Pause(0x80, 0);
            Timer_SetElapsed(G32(0x101354d0), (int)G32(0x1015dc78));
        }
    }
    G32(0x1015dc80) = G32(0x1015dc78) - G32(0x1015dc98);
    if (G32(0x1015ec58) == 0) {
        if (G32(0x1015dc8c) != 0) {
            G32(0x1015dc80) = G32(0x1015dc80) << 3;
            G32(0x1015dc78) += G32(0x1015dc80);
            Timer_SetElapsed(G32(0x101354d0), (int)G32(0x1015dc78));
        } else if (G32(0x1015dc90) != 0) {
            GS32(0x1015dc80) = GS32(0x1015dc80) >> 2;
            G32(0x1015dc78) += G32(0x1015dc80);
            Timer_SetElapsed(G32(0x101354d0), (int)G32(0x1015dc78));
        }
    }
    if (GS32(0x1015dc80) <= 0) {
        G32(0x1015dc80) = 0;
        G32(0x1015dc78) = G32(0x1015dc98);
    }
    G32(0x1015dc98) = G32(0x1015dc78);
}
