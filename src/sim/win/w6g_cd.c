#include "w6g.h"
#include "w4u.h"
#include "w4w.h"
#include "../util/access.h"

// FUNCTION: HGSIM 0x1009e1ef
int __cdecl Cd_Open(void)
{
    G32(0x1017d060) = 0;
    G32(0x1017d064) = 0;
    if (W6G_CD_INIT() != 0)
        return 0;
    G32(0x1017d08c) = 1;
    G32(0x1017d078) = (uint32_t)Cd_GetStatus();
    switch (G32(0x1017d078)) {
    case 1:
        G32(0x1017d08c) = 0;
        return 0;
    case 2:
        Cd_LoadTracks(SIM_PTR(uint32_t, 0x1017d080));
        break;
    case 3:
        Cd_LoadTracks(SIM_PTR(uint32_t, 0x1017d080));
        break;
    case 4:
        Cd_LoadTracks(SIM_PTR(uint32_t, 0x1017d080));
        Cd_ReadPosition(SIM_PTR(uint32_t, 0x1017d068));
        break;
    }
    Win_AuxSetVolume((int)G32(0x101321bc));
    return 1;
}

// FUNCTION: HGSIM 0x1009e39d
void __cdecl Cd_OpenDoor(void)
{
    uint32_t parms[3];      /* MCI_SET_PARMS, not filled in (as in the original) */

    if (SIM_API(W6gMciSendCommandFn, 0x10ac5850)(G32(0x1014c9b0), 0x80d, 0x100, parms) != 0)
        Cd_Shutdown();
}
