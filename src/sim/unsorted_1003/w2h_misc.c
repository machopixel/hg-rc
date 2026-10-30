#include "w2h.h"
#include "../util/access.h"

// FUNCTION: HGSIM 0x10033540
int32_t __cdecl Sim_10033540(const int32_t *p)
{
    if (p != 0)
        return *p;
    return -1;
}

// FUNCTION: HGSIM 0x10036820
int __cdecl Sim_10036820(uint32_t a, int32_t b)
{
    /* For b == 1 the original also reads the dword at 0x109e3d20, but returns 1 whatever it holds. */
    if (a == 0)
        return 0;
    return b == 0 || b == 1 || b == 2;
}

// FUNCTION: HGSIM 0x1003830e
int __cdecl Sim_1003830e(int32_t start, int32_t count, void *dest)
{
    int32_t i;

    if (dest == 0 || start < 0 || start > 0xff || count <= 0 || count > 0x100 - start)
        return -1;
    for (i = 0; i < count; i++) {
        uint8_t *from = GBYTES(0x1015e6a0 + (uint32_t)(start + i) * 3);
        uint8_t *to = (uint8_t *)dest + (uint32_t)i * 3;

        U16(to, 0) = U16(from, 0);
        U8(to, 2) = U8(from, 2);
    }
    return 0;
}

// FUNCTION: HGSIM 0x1003b45d
void __cdecl Sim_1003b45d(int32_t index, uint32_t value)
{
    G32(0x109b9e68 + (uint32_t)index * 0x7e) = value;
}
