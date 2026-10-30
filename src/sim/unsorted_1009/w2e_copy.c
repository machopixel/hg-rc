#include "w2e.h"
#include "../unsorted_100f/w2e.h"
#include "../util/access.h"

// FUNCTION: HGSIM 0x1009cfbd
void *__cdecl Sim_1009cfbd(void *dst, const void *src)
{
    return Mem_Move(dst, src, 0x38);
}

// FUNCTION: HGSIM 0x1009d84e
void __cdecl Sim_1009d84e(void *obj, const char *s1, uint32_t value, const char *s2)
{
    int k;

    if (obj == 0 || U32(obj, 0) == 0)
        return;
    k = S32(U32(obj, 0), 4);
    if (k > 0x19)
        return;
    G32(0x1017cd50 + (uint32_t)k * 0x16u) = value;
    Str_Copy((char *)GBYTES(0x1017cd54 + (uint32_t)k * 0x16u), s1);
    Str_Copy((char *)GBYTES(0x1017cd5d + (uint32_t)k * 0x16u), s2);
}
