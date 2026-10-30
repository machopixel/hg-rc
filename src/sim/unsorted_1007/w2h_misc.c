#include "w2h.h"
#include "../util/access.h"

// FUNCTION: HGSIM 0x1007655f
uint32_t __cdecl Sim_1007655f(uint32_t key)
{
    uint32_t node = G32(0x1015dc14);

    while (node != 0 && U32(node, 0) != key)
        node = U32(node, 0x14);
    if (node != 0)
        return U32(node, 0xc);
    return 0;
}

/* Sets bit 30 of *link when it names a live object whose dword +8 is id. */
static int MarkLinked(uint32_t link, uint32_t linkAddr, int32_t id)
{
    int32_t n;
    uint32_t other;

    if ((link & 0x80000000u) != 0 || (link & 0x40000000u) != 0)
        return 0;
    n = (int16_t)(link & 0xffff);
    if (n < 0 || n >= 0x19 || n >= GS32(0x1015dc64))
        return 0;
    other = PTRTAB(n);
    if (other == 0 || S32(other, 8) != id)
        return 0;
    U32(linkAddr, 0) = U32(linkAddr, 0) | 0x40000000u;
    return 1;
}

// FUNCTION: HGSIM 0x10079c86
int __cdecl Sim_10079c86(const uint8_t *a, int32_t b)
{
    uint32_t index = a[0];
    uint32_t kind = (uint32_t)a[1] << 8;
    uint32_t addr;

    if (kind == 0x200) {
        addr = PTRTAB(index) + 0x150;
        return MarkLinked(U32(addr, 0), addr, b);
    }
    if (kind == 0x400) {
        addr = (uint32_t)(uintptr_t)GBYTES(0x109e9ba0 + index * 0x50 + 0x4c);
        return MarkLinked(U32(addr, 0), addr, b);
    }
    return 0;
}

// FUNCTION: HGSIM 0x1007c884
void __cdecl Sim_1007c884(void)
{
    G32(0x1037305c) = (uint32_t)(GS32(0x1015dc78) / 0xb5);
}

// FUNCTION: HGSIM 0x1007d8b0
int __cdecl Sim_1007d8b0(void)
{
    return 1;
}

// FUNCTION: HGSIM 0x1007dba3
int __cdecl Sim_1007dba3(void)
{
    return 0;
}
