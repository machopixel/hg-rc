#include "w5c.h"
#include "../unsorted_100f/w2e.h"
#include "../util/access.h"

// FUNCTION: HGSIM 0x1005264d
void __cdecl Sim_1005264d(int a, int b)
{
    int k;

    if (a >= 0x10 || b >= 0x20)
        return;
    if (a == (int)G32(0x1013bb40))
        SIM_CALL_100203b8(b);
    *(u32a *)(GBYTES(0x109b7d58) + (int32_t)((uint32_t)a * 0x6c)) = (uint32_t)b;
    for (k = 0; k < 0x1d; k++)
        ((u32a *)(GBYTES(0x109b8420) + (int32_t)((uint32_t)a * 0x74)))[k] = ((u32a *)(GBYTES(0x109b68e0) + (int32_t)((uint32_t)b * 0x74)))[k];
}

// FUNCTION: HGSIM 0x1005af06
void *__cdecl Sim_1005af06(void *node, const char *name)
{
    void *found = 0;

    if (node != 0) {
        if (Str_Compare((const char *)(uintptr_t)U32(node, 0x90), name) == 0) {
            found = node;
        } else {
            found = Sim_1005af06((void *)(uintptr_t)U32(node, 4), name);
            if (found == 0)
                found = Sim_1005af06((void *)(uintptr_t)U32(node, 8), name);
        }
    }
    return found;
}
