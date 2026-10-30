#include "w5b.h"
#include "../unsorted_1008/w3s.h"
#include "../unsorted_1008/w4v.h"
#include "../util/access.h"

// FUNCTION: HGSIM 0x100d3491
void __cdecl Sim_100d3491(void)
{
    if (G32(0x1017e65c) != 0) {
        Sim_1008c803(SIM_PTR(void, 0x1017e620));
        G32(0x1017e65c) = G32(0x1017e65c) - 1;
        if (G32(0x1017e65c) == 0) {
            G32(0x10152420) = G32(0x10152428);
            if (G32(0x1017e658) != 0)
                SIM_CALL_100d3510((int)G32(0x1015242c), G32(0x1017e658), 0);
            else
                G32(0x1017e650) = 1;
        }
    }
    if (G32(0x1017e660) != 0)
        Sim_1008c5e0(SIM_PTR(void, 0x1017e640));
}
