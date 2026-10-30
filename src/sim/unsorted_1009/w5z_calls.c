#include "w5z.h"
#include "../simmem.h"
#include "../util/access.h"

typedef uint32_t(__cdecl *Fn10098721)(uint32_t a, uint32_t b);
typedef void(__cdecl *Fn1008a94d)(uint32_t v);

// FUNCTION: HGSIM 0x10098710
uint32_t __cdecl Sim_10098710(void)
{
    return SIM_FN(Fn10098721, 0x10098721)(0, 0);
}

// FUNCTION: HGSIM 0x10098a5d
void __cdecl Sim_10098a5d(uint32_t unused, uint32_t v)
{
    (void)unused;
    SIM_FN(Fn1008a94d, 0x1008a94d)(v);
    U32(G32(0x1015d1b8), 0x20) = v;
}
