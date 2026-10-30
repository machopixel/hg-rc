#include "w2i.h"
#include "../util/access.h"

// FUNCTION: HGSIM 0x100a9b7f
uint32_t __cdecl Sim_100a9b7f(void *obj)
{
    return U32(obj, 0x90);
}

// FUNCTION: HGSIM 0x100aabfd
void *__cdecl Sim_100aabfd(void *node)
{
    uint32_t last = 0;
    uint32_t cur = U32(node, 4);

    while (cur != 0) {
        last = cur;
        cur = U32(cur, 8);
    }
    return (void *)(uintptr_t)last;
}
