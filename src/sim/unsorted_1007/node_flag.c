#include "fns.h"
#include "../simmem.h"
#include "../view/view.h"

// FUNCTION: HGSIM 0x10074bfa
int __cdecl Sim_10074bfa(int32_t key)
{
    uint32_t *node;

    if (*SIM_PTR(int32_t, 0x1015dbcc) != 0)
        return 0;
    node = (uint32_t *)Sim_1007652f(key);
    if (node == 0)
        return 0;
    node[2] = 1;
    return 1;
}

// FUNCTION: HGSIM 0x100751e4
void __cdecl Sim_100751e4(int32_t key)
{
    uint32_t *node = (uint32_t *)Sim_1007652f(key);

    if (node != 0)
        node[2] = 0;
}
