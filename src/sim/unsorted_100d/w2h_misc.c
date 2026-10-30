#include "w2h.h"
#include "../util/access.h"

// FUNCTION: HGSIM 0x100d624d
int __cdecl Sim_100d624d(const void *obj)
{
    uint32_t e;

    if (S32(obj, 0x4fc) == -1)
        return 0;
    e = U32(obj, 0x84) + (uint32_t)S32(obj, 0x4fc) * 0x128;
    if (GS32(0x10145534 + U32(e, 4) * 0xd6) > 0 && S32(e, 0x20) > 0) {
        S32(e, 0x20) = 0;
        S32(e, 8) = -1;
        return 1;
    }
    return 0;
}
