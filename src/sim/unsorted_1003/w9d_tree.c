#include "w9d.h"
#include "../util/access.h"

// FUNCTION: HGSIM 0x1003218b
int __cdecl Sim_1003218b(void *node)
{
    uint32_t size;
    int i;

    if (node == 0)
        return 0;
    size = U32(node, 0x18) * 4 + 0x2c;
    for (i = 0; i < 4; i++)
        size += (uint32_t)Sim_1003218b((void *)(uintptr_t)U32(node, 0x1c + i * 4));
    return (int)size;
}
