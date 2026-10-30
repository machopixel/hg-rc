#include "w7o.h"
#include "w6f.h"
#include "../util/access.h"
#include "../view/view.h"
#include "../win/w4u.h"
#include "../unsorted_100f/w6i.h"
#include "../unsorted_1010/w6k.h"

#define MODE GS32(0x1015d1ac)
#define REQUEST GS32(0x1015d1cc)

/*
 * The common part of the two switches; `view` is stored to [0x1015d1c8] after the old value went to
 * [0x1015d104]. Returns -1 when nothing was applied, else what Sim_1009b8d8 returned.
 */
static int32_t SwitchView(uint32_t view, int32_t flip, int *applied)
{
    uint32_t entry, sub;

    *applied = 0;
    if (MODE <= 2) {
        Sim_100999fc();
        if (MODE > 2)
            return 0;
        if (MODE != 0) {
            G32(0x1015d104) = G32(0x1015d1c8);
            G32(0x1015d1c8) = view;
        }
        entry = SIM_PTR(u32a, 0x1014c4c8)[MODE];
        if (entry == 0)
            return 0;
        sub = U32(entry, 0x14);
        if (sub == 0)
            return 0;
        if (G32(0x1015d104) != G32(0x1015d1c8))
            Sim_1008f630((void **)(uintptr_t)sub);
        *applied = 1;
        return Sim_1009b8d8(flip, (uint32_t *)(uintptr_t)entry, (const uint32_t *)(uintptr_t)sub, 0);
    }
    if (MODE == 4 && REQUEST <= 2)
        Sim_100999fc();
    return 0;
}

// FUNCTION: HGSIM 0x1009b99b
void __cdecl Sim_1009b99b(void)
{
    int applied;

    if (SwitchView(1, 0, &applied) == 0 && applied)
        W7O_CALL_10099c36();
}

// FUNCTION: HGSIM 0x1009bc94
void __cdecl Sim_1009bc94(void)
{
    int applied;

    SwitchView(3, 1, &applied);
}

// FUNCTION: HGSIM 0x1009e5d0
int __cdecl Cd_PollDisc(void)
{
    int status;
    int tries;

    if (G32(0x1017d08c) == 0)
        return 0;
    status = Cd_GetStatus();
    if ((uint32_t)status == G32(0x1017d078))
        return 0;
    if (status == 1) {
        Cd_FreeTracks(SIM_PTR(uint32_t, 0x1017d080));
    } else if (status == 2 && G32(0x1017d078) == 1) {
        for (tries = 0; tries < 3; tries++) {
            if (Cd_LoadTracks(SIM_PTR(uint32_t, 0x1017d080)) == 0)
                break;
            Sim_1010d2f0(7);
        }
        if (tries == 3)
            Proc_Exit(0);
    }
    G32(0x1017d078) = (uint32_t)status;
    return 1;
}
