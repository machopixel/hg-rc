#include "w2f.h"
#include "../util/access.h"

#define ADDR(p) ((uint32_t)(uintptr_t)(p))

// FUNCTION: HGSIM 0x10027cce
void __cdecl Sim_10027cce(void *node)
{
    if (S16(node, 2) == 1)
        return;
    S16(node, 2) = 1;
    if (U32(node, 0xc) != 0)
        U32(U32(node, 0xc), 0x10) = U32(node, 0x10);
    if (U32(node, 0x10) != 0)
        U32(U32(node, 0x10), 0xc) = U32(node, 0xc);
    if (ADDR(node) == G32(0x109f13f0))
        G32(0x109f13f0) = U32(node, 0xc);
    if (ADDR(node) == G32(0x109f13f4))
        G32(0x109f13f4) = U32(node, 0x10);
    U32(node, 0xc) = 0;
    U32(node, 0x10) = 0;
}

// FUNCTION: HGSIM 0x10027f2e
void __cdecl Sim_10027f2e(void)
{
}

// FUNCTION: HGSIM 0x10028a07
void __cdecl Sim_10028a07(void *p, uint32_t v)
{
    U32(p, 0x30) = v;
}

// FUNCTION: HGSIM 0x10028a37
void __cdecl Sim_10028a37(void *p, uint16_t a, uint16_t b, uint16_t c, uint16_t d)
{
    U16(p, 0xfc) = a;
    U16(p, 0xfe) = b;
    U16(p, 0x100) = c;
    U16(p, 0x102) = d;
}
