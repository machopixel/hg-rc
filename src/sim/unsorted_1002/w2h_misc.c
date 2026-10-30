#include "w2h.h"
#include "../unsorted_1008/fns.h"
#include "../view/w2d.h"
#include "../util/access.h"

// FUNCTION: HGSIM 0x10028289
void __cdecl Sim_10028289(void)
{
}

// FUNCTION: HGSIM 0x10028a92
void __cdecl Sim_10028a92(void *obj)
{
    U16(obj, 4) = 0;
}

// FUNCTION: HGSIM 0x1002d244
int __cdecl Sim_1002d244(uint32_t a, uint32_t b, uint32_t c, uint32_t d)
{
    (void)a;
    (void)b;
    (void)c;
    (void)d;
    return 0;
}

// FUNCTION: HGSIM 0x1002d890
void __cdecl Sim_1002d890(void *obj, int32_t state)
{
    S32(obj, 0x34) = state;
    if (state == 4)
        Sim_1008a37f(obj);
    else
        Sim_1008a220((uint16_t *)obj);
}
