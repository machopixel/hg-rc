#include "w9e.h"
#include "../util/access.h"

// FUNCTION: HGSIM 0x100dc2d7
int __cdecl GameThing_NextIndex(void)
{
    int result = -1;

    if (GS32(0x1015dc68) < 0xfe) {
        result = GS32(0x1015dc68);
        G32(0x1015dc68) = G32(0x1015dc68) + 1;
    } else {
        W9E_SYSERR(0x2f, SIM_PTR(const char, 0x10152500), 0);
    }
    return result;
}
