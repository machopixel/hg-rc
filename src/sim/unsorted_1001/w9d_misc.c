#include "w9d.h"
#include "../util/access.h"
#include "../win/w4u.h"

// FUNCTION: HGSIM 0x10017b3e
void __cdecl Sim_10017b3e(void *obj, uint32_t value)
{
    uint32_t block = U32(obj, 0x28);

    U32(obj, 0x23e) = value;
    U32(block, 0x58c) = 1;
}

// FUNCTION: HGSIM 0x1001edd0
uint32_t __cdecl Sim_1001edd0(int which)
{
    uint32_t result = 0;

    if (which < 0 || which >= 5)
        return 0;
    switch (which) {
    case 0:
        result = G32(0x101321b4);
        break;
    case 1:
        result = G32(0x101321b8);
        break;
    case 2:
        result = G32(0x101321bc);
        G32(0x101321c0) = G32(0x101321c0) | 8;
        break;
    case 3:
        result = G32(0x101321b0);
        break;
    case 4:
        result = (G32(0x101321c0) & 2) != 0 ? 1 : 0;
        break;
    }
    return result;
}
