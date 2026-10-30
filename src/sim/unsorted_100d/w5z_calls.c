#include "w5z.h"
#include "../simmem.h"
#include "../util/access.h"
#include "../unsorted_1008/w4w.h"

typedef uint32_t(__cdecl *Fn100d2306)(uint32_t a);
typedef uint32_t(__cdecl *Fn100d255e)(uint32_t a);
typedef void(__cdecl *Fn100d3427)(uint32_t a);
typedef uint32_t(__cdecl *Fn100d3510)(uint32_t a, uint32_t b, uint32_t c);
typedef void(__cdecl *Fn100bb2b0)(uint32_t a, uint32_t b);

// FUNCTION: HGSIM 0x100d18e8
uint32_t __cdecl Tree_CountNodes(void *node)
{
    uint32_t first, second;

    if (node == 0)
        return 0;
    first = Tree_CountNodes(BYTES(U32(node, 0xc)));
    second = Tree_CountNodes(BYTES(U32(node, 8)));
    return first + second + 1;
}

// FUNCTION: HGSIM 0x100d25ce
uint32_t __cdecl Sim_100d25ce(uint32_t a)
{
    SIM_FN(Fn100d2306, 0x100d2306)(a);
    return SIM_FN(Fn100d255e, 0x100d255e)(a);
}

// FUNCTION: HGSIM 0x100d33ec
void __cdecl Sim_100d33ec(void)
{
    if (G32(0x1017e650) == 0 || GS32(0x1017e65c) > 0)
        return;
    G32(0x1017e650) = 0;
    SIM_FN(Fn100d3427, 0x100d3427)(G32(0x10152424));
    G32(0x10152420) = G32(0x10152424);
}

// FUNCTION: HGSIM 0x100d36d6
uint32_t __cdecl Sim_100d36d6(uint32_t a, uint32_t b, uint32_t c)
{
    return SIM_FN(Fn100d3510, 0x100d3510)(G32(0x1017e654) + a, b, c);
}

// FUNCTION: HGSIM 0x100d379f
void __cdecl Sim_100d379f(void)
{
    if (G32(0x1017e660) == 0)
        return;
    Sim_1008c6b6(GBYTES(0x1017e640));
    G32(0x1017e660) = 0;
    SIM_FN(Fn100bb2b0, 0x100bb2b0)(G32(0x1017e664), G32(0x1014ced4));
    G32(0x1017e664) = 0;
    Sim_100d33ec();
}
