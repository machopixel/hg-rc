#include "w5c.h"
#include "../util/util.h"
#include "../unsorted_1001/w5c.h"
#include "../util/access.h"

// FUNCTION: HGSIM 0x100c00c7
void __cdecl Sim_100c00c7(void *obj, uint32_t a, int16_t b, uint32_t c)
{
    uint32_t l4;
    uint32_t lc;
    uint32_t l8;
    int idx;

    SIM_CALL_100c0158(a, b, &l4, &lc, &l8, c);
    idx = Sim_100ca350(U32(obj, 4), l4, l8, lc);
    if (idx == -1)
        return;
    U16(GBYTES(0x109a8664), (uint32_t)idx * 0x54) = (uint16_t)(U16(GBYTES(0x109a8664), (uint32_t)idx * 0x54) | 1);
    U32(GBYTES(0x109a8648), (uint32_t)idx * 0x54) = U32(obj, 4) | 0x200;
    Sim_100159cf(obj, 0x100);
}
