#include "w2h.h"
#include "../util/access.h"

// FUNCTION: HGSIM 0x100071f1
void __cdecl Sim_100071f1(void)
{
}

// FUNCTION: HGSIM 0x100089a0
uint32_t __cdecl Sim_100089a0(void)
{
    return G32(0x1015d09c);
}

// FUNCTION: HGSIM 0x1000945e
uint32_t __cdecl Sim_1000945e(const void *obj)
{
    return U16(obj, 0x24);
}

// FUNCTION: HGSIM 0x100094df
void __cdecl Sim_100094df(const void *obj, int32_t *a, int32_t *b)
{
    uint32_t link = U32(obj, 0x30);

    if (link == 0)
        return;
    if (a != 0)
        *a = S16(link, 4);
    if (b != 0)
        *b = S16(link, 6);
}
