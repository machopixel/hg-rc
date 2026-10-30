#include "w2j.h"
#include "../util/access.h"
#include "../unsorted_1000/w2j.h"

// FUNCTION: HGSIM 0x100107ce
void __cdecl Sim_100107ce(int arg)
{
    if (arg != 0) {
        uint32_t address = G32(0x1013bb40) * 0x4fc3au + (uint32_t)arg * 0x13fu + 0x1047383au;

        U32(SIM_PTR(uint8_t, address), 2) = 0x10000;
    }
}

// FUNCTION: HGSIM 0x100109e7
int __cdecl Sim_100109e7(int mode)
{
    int first = Sim_1000fe60(1);
    int second = Sim_1000fe60(2);

    if (mode == 0) {
        if (first != 0 && second == 0)
            mode = 1;
        else if (first == 0 && second != 0)
            mode = 2;
        else if (first == 0 && second == 0)
            return -1;
    } else if (mode == 1 && first == 0) {
        return -1;
    } else if (mode == 2 && second == 0) {
        return -1;
    }
    return mode;
}

// FUNCTION: HGSIM 0x100125d4
void __cdecl Sim_100125d4(void)
{
}

// FUNCTION: HGSIM 0x10017892
uint32_t __cdecl Sim_10017892(void)
{
    uint32_t record = PTRTAB(G32(0x1017b8c8));

    return U32(SIM_PTR(uint8_t, 0x109b7d44u + U32(record, 8) * 0x6cu), 0);
}
