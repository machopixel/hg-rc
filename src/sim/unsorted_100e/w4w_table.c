#include "w4w.h"
#include "../util/access.h"

// FUNCTION: HGSIM 0x100e469b
void __cdecl Sim_100e469b(uint32_t index)
{
    uint32_t table;

    if ((index & 0x80) == 0) {
        table = 0x10153280;
    } else {
        index ^= 0x80;
        table = 0x10153180;
    }
    G32(table + index * 4u) = 0;
}
