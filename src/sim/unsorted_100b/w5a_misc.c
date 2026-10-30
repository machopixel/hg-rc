#include "w5a.h"
#include "../util/access.h"

// FUNCTION: HGSIM 0x100b4342
void __cdecl Sim_100b4342(int a, int b, int c)
{
    uint8_t *entry;
    uint8_t *other;

    if (a < 0 || a >= GS32(0x1017d218))
        return;
    entry = GBYTES(0x101a2100) + (uint32_t)a * 0x88u;
    U32(entry, 0x18) |= 0x400;
    U32(entry, 0x1c) |= 0x200;
    S32(entry, 0x20) = (int16_t)b;
    S32(entry, 0x24) = (int16_t)c;
    if (b > -1) {
        other = GBYTES(0x101a2100) + (uint32_t)b * 0x88u;
        U32(other, 0x18) |= 0x800;
        W5A_CALL_100b469d();
    }
}
