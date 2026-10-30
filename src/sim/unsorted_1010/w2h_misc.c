#include "w2h.h"
#include "../simmem.h"
#include "../util/access.h"
#include <string.h>

// FUNCTION: HGSIM 0x10100390
void __cdecl Sim_10100390(uint32_t value)
{
    G32(0x101596f8) = value;
}

// FUNCTION: HGSIM 0x10106900
uint32_t __cdecl Sim_10106900(uint32_t addr, uint32_t *node, uint32_t *page)
{
    uint8_t *head = GBYTES(0x10159e38);
    uint8_t *n = head;
    uint32_t start;

    for (;;) {
        if (addr > U32(n, 0x10) && addr < U32(n, 0x14))
            break;
        n = BYTES(U32(n, 0));
        if (n == head)
            return 0;
    }
    if ((addr & 0xf) != 0 || (addr & 0xfff) < 0x100)
        return 0;
    *node = (uint32_t)(uintptr_t)n;
    start = addr & 0xfffff000u;
    *page = start;
    return start + 8 + ((addr - start - 0x100) >> 4);
}

// FUNCTION: HGSIM 0x10107fd0
int __cdecl Sim_10107fd0(void)
{
    memset(GBYTES(0x1017eac0), 0, 0x101);
    G32(0x1017ebc4) = 0;
    G32(0x1017ebc8) = 0;
    G32(0x1017ebd0) = 0;
    G32(0x1017ebd4) = 0;
    G32(0x1017ebd8) = 0;
    return 0;
}

// FUNCTION: HGSIM 0x101087b0
uint32_t __cdecl Sim_101087b0(uint32_t flags)
{
    uint32_t r = 0;
    uint32_t bits8_9 = flags & 0x300;
    uint32_t bits16_17 = flags & 0x30000;

    if (flags & 0x10)
        r = 1;
    if (flags & 8)
        r |= 4;
    if (flags & 4)
        r |= 8;
    if (flags & 2)
        r |= 0x10;
    if (flags & 1)
        r |= 0x20;
    if (flags & 0x80000)
        r |= 2;
    if (bits8_9 == 0x100)
        r |= 0x400;
    else if (bits8_9 == 0x200)
        r |= 0x800;
    else if (bits8_9 == 0x300)
        r |= 0xc00;
    if (bits16_17 == 0)
        r |= 0x300;
    else if (bits16_17 == 0x10000)
        r |= 0x200;
    if (flags & 0x40000)
        r |= 0x1000;
    return r;
}
