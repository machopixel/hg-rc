#include "w9g.h"
#include "../util/access.h"
#include "../view/view.h"

// FUNCTION: HGSIM 0x10073041
void __cdecl Sim_10073041(void *obj, uint32_t value)
{
    U32(U32(obj, 0x28), 0x4fc) = value;
}

// FUNCTION: HGSIM 0x100766ca
int32_t __cdecl Sim_100766ca(int32_t key)
{
    int32_t value = 0;
    void *node = Sim_1007652f(key);

    if (node != 0)
        value = S32(node, 4);
    return value;
}
