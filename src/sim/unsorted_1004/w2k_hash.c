#include "w2k.h"
#include "../util/access.h"

// FUNCTION: HGSIM 0x1004e582
unsigned __cdecl Sim_1004e582(unsigned key, unsigned byte)
{
    uint32_t slot;
    uint32_t entry;

    if (key == 0xFFFF)
        return byte & 0xFF;
    slot = (((byte & 0xFF) << 8) ^ key) % 9973;
    for (;;) {
        entry = U32(G32(0x1015F35C), slot * 4);
        if (entry == 0xFFFF)
            return 0xFFFF;
        if (U32(G32(0x1015F358), entry * 4) == key && U8(G32(0x1015F354), entry) == (byte & 0xFF))
            return entry;
        slot = (slot + 0x7F7) % 9973;
    }
}
