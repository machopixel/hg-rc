#include "w5z.h"
#include "../simmem.h"
#include "../util/access.h"

typedef uint32_t(__cdecl *Fn100c4b67)(void *p);
typedef uint32_t(__cdecl *Fn100c4a1f)(void *p);
typedef uint32_t(__cdecl *Fn100ca670)(uint32_t a, uint32_t b, uint32_t flags);

// FUNCTION: HGSIM 0x100c4d63
uint32_t __cdecl Sim_100c4d63(void *p)
{
    uint32_t result;

    if (p == 0)
        return 0;
    result = SIM_FN(Fn100c4b67, 0x100c4b67)(p);
    if (result == 0)
        result = SIM_FN(Fn100c4a1f, 0x100c4a1f)(p);
    return result;
}

// FUNCTION: HGSIM 0x100cbdb8
uint32_t __cdecl Sim_100cbdb8(uint32_t a, uint32_t b, int c)
{
    uint32_t flags = 1;

    if (c != 0)
        flags |= 0x100;
    return SIM_FN(Fn100ca670, 0x100ca670)(a, b, flags);
}

// FUNCTION: HGSIM 0x100cbdea
uint32_t __cdecl Sim_100cbdea(uint32_t a)
{
    return SIM_FN(Fn100ca670, 0x100ca670)(PTRTAB(G32(0x1017b8c8)), a, 4);
}
