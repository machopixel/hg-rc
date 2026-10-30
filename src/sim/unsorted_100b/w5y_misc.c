#include "w5y.h"
#include "../util/access.h"

// FUNCTION: HGSIM 0x100b7834
uint32_t __cdecl Sim_100b7834(const uint32_t *table, uint32_t key)
{
    uint32_t search_key = key;
    const uint32_t *found;

    if (table != 0) {
        found = SIM_FN(const uint32_t *(__cdecl *)(const void *, const void *, uint32_t, uint32_t, const void *),
                       0x10100620)(&search_key, (const void *)(uintptr_t)table[1], table[0], 8, GBYTES(0x100b6b87));
        if (found != 0)
            key = found[1];
    }
    return key;
}
