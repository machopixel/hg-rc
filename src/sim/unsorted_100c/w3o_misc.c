#include "w3o.h"
#include "../unsorted_1004/fns.h"
#include "../unsorted_1008/w3o.h"
#include "../util/util.h"
#include "../util/w3o.h"

// FUNCTION: HGSIM 0x100c476c
int __cdecl Sim_100c476c(void *obj)
{
    int found = 0;
    int owner = Sim_10052be7((int)U32(obj, 4));
    int i;

    for (i = 0; i < 0x200; i++) {
        uint8_t *rec = GBYTES(0x1096fc80u + (uint32_t)i * 0x156);
        uint32_t px, py, pz;
        float dx, dy, dz;

        if (owner == Sim_10052be7((int)U32(rec, 0x50)) || U32(rec, 0x3c) == 0)
            continue;
        Sim_100a9cfb((void *)(uintptr_t)U32(rec, 0x3c), &px, &py, &pz);
        dx = (float)((double)FL(obj, 0x58) - Float_OfBits(px));
        dy = (float)((double)FL(obj, 0x5c) - Float_OfBits(py));
        dz = (float)((double)FL(obj, 0x60) - Float_OfBits(pz));
        if (Sim_10084a94(dx, dy, dz) > GFL(0x1012bbc0))
            continue;
        if (((double)FL(rec, 0x18) * dx + (double)FL(rec, 0x1c) * dy) + (double)FL(rec, 0x20) * dz > GFL(0x1012bb18))
            found = 1;
    }
    return found;
}

// FUNCTION: HGSIM 0x100cc863
void __cdecl Sim_100cc863(void *list, void *node)
{
    ((void(__cdecl *)(int, int, uint32_t, uint32_t))(uintptr_t)U32(node, 0))(-1, 0, G32(0x1015dc78), U32(node, 8));
}

// FUNCTION: HGSIM 0x100cc887
void __cdecl Sim_100cc887(void *list)
{
    uint32_t node;

    for (node = U32(list, 0); node != 0; node = U32(node, 0x14))
        Sim_100cc863(list, (void *)(uintptr_t)node);
}
