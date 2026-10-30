#include "w2f.h"
#include "../util/access.h"

// FUNCTION: HGSIM 0x100d3861
void __cdecl Sim_100d3861(uint32_t v)
{
    G32(0x1017e654) = v;
    G32(0x10152424) = v;
}

// FUNCTION: HGSIM 0x100d60b6
int __cdecl Sim_100d60b6(void *obj, int v)
{
    int found = 0;
    int32_t i;

    if (v < 0 || v >= 3)
        return 0;
    if (S32(obj, 0x4f8) <= 0)
        return 0;
    if (v == S32(U32(obj, 0x84) + (uint32_t)S32(obj, 0x4fc) * 0x128, 0xc))
        return 1;
    for (i = 0; i < 8 && !found; i++) {
        uint32_t rec = U32(obj, 0x84) + (uint32_t)i * 0x128;
        int32_t kind = S32(rec, 4);
        uint32_t entry;

        if (kind < 0)
            continue;
        entry = 0x10145508 + (uint32_t)kind * 0xd6;
        if (S32(rec, 0) != -1 && S32(rec, 0xc) == v && S32(rec, 8) != -1 && (U32(GBYTES(entry), 8) & 4) != 0) {
            found = 1;
            S32(obj, 0x4fc) = i;
        }
    }
    return found;
}

// FUNCTION: HGSIM 0x100d6200
int __cdecl Sim_100d6200(void *obj)
{
    int r = -1;

    if (S32(obj, 0x4fc) != -1)
        r = S32(U32(obj, 0x84) + (uint32_t)S32(obj, 0x4fc) * 0x128, 8) == 1;
    return r;
}
