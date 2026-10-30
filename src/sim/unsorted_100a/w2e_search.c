#include "w2e.h"
#include "../unsorted_100f/w2e.h"
#include "../util/access.h"

// FUNCTION: HGSIM 0x100a7da3
uint32_t __cdecl Sim_100a7da3(int index, const char *key)
{
    uint8_t *rec = BYTES(G32(0x101c65e6 + (uint32_t)index * 0x14au));
    uint16_t count = U16(rec, 0xc);
    uint8_t *entries = rec + 0xe;
    uint16_t i = 0;
    int found = 0;

    while (!found && i < count) {
        if (Str_CompareN((const char *)entries + (uint32_t)i * 0x18u, key, 4) == 0)
            found = 1;
        else
            i++;
    }
    return found ? i : 0xFFFF;
}
