#include "w5z.h"
#include "../simmem.h"
#include "../util/access.h"

typedef uint32_t(__cdecl *Fn100d3510)(uint32_t a, uint32_t b, uint32_t c);
typedef uint32_t(__cdecl *Fn1006519d)(uint32_t a, uint32_t b, uint32_t c);

// FUNCTION: HGSIM 0x10061b62
uint32_t __cdecl Sim_10061b62(void)
{
    return SIM_FN(Fn100d3510, 0x100d3510)(0x11, 0x16a, 1);
}

// FUNCTION: HGSIM 0x1006535f
uint32_t __cdecl Sim_1006535f(uint32_t a, uint32_t b, uint32_t c, const void *p)
{
    return SIM_FN(Fn1006519d, 0x1006519d)(a + U32(p, 4), b + U32(p, 8), c);
}
