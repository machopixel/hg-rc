#include "w5b.h"
#include "../util/access.h"

// FUNCTION: HGSIM 0x1003cb21
void __cdecl Sim_1003cb21(int index)
{
    int32_t second;

    if (GS32(0x1015ebb0) != 1 || index == GS32(0x1013abb8))
        return;
    if (GS32(0x109b9e08) == 1) {
        second = 0xb5;
        G32(0x109b9e08) = 0;
    } else {
        second = GS32(0x1013ab9cu + (uint32_t)index * 8u);
    }
    if (GS32(0x1015eb98) == 2) {
        G32(0x1015eb98) = G32(0x1015eb98) + 1;
        second = 0x16a;
    }
    SIM_CALL_100d3830(G32(0x1013ab98u + (uint32_t)index * 8u), second);
    G32(0x1013abb8) = (uint32_t)index;
}
