#include "w4u.h"
#include "../simmem.h"
#include "../util/access.h"
#include "../win/w4u.h"

// FUNCTION: HGSIM 0x1001f367
void __cdecl Sim_1001f367(void)
{
    if (G32(0x1015d1bc) != 0) {
        if (GS32(0x101321a8) != -1 && (G32(0x101321c0) & 8) != 0)
            Cd_TogglePause();
        G32(0x1015d1bc) = 0;
    }
}

// FUNCTION: HGSIM 0x1001f335
void __cdecl Sim_1001f335(void)
{
    if (G32(0x1015d1bc) == 0) {
        if (GS32(0x101321a8) != -1 && (G32(0x101321c0) & 8) != 0)
            Cd_TogglePause();
        G32(0x1015d1bc) = 1;
    }
}

// FUNCTION: HGSIM 0x1001f399
void __cdecl Sim_1001f399(void)
{
    if (GS32(0x101321a8) != -1)
        Cd_StopWait();
}

// FUNCTION: HGSIM 0x1001f4d3
void __cdecl Sim_1001f4d3(void)
{
    Sim_1001f367();
}
