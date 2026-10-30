#include "w5a.h"
#include "../util/access.h"

// FUNCTION: HGSIM 0x100579ad
void __cdecl Sim_100579ad(const uint32_t *a, const uint32_t *b)
{
    int i;

    if (G32(0x101668a4) != 0)
        return;
    G32(0x101668a4) = 1;
    if (a != 0) {
        for (i = 0; i < 0x7d; i++)
            SIM_PTR(u32a, 0x10143808)[i] = a[i];
    }
    if (b != 0) {
        for (i = 0; i < 0x7d; i++)
            SIM_PTR(u32a, 0x10165f60)[i] = W5A_CALL_100b7834(G32(0x1017d594), b[i]);
    }
}

// FUNCTION: HGSIM 0x1005ae79
void *__cdecl Sim_1005ae79(void *node, const char *key)
{
    void *found = 0;

    if (node != 0) {
        if (W5A_STRSTR((const char *)(uintptr_t)U32(node, 0x90), key) != 0) {
            found = node;
        } else {
            found = Sim_1005ae79((void *)(uintptr_t)U32(node, 4), key);
            if (found == 0)
                found = Sim_1005ae79((void *)(uintptr_t)U32(node, 8), key);
        }
    }
    return found;
}
