#include "w5x.h"
#include "../util/access.h"

// FUNCTION: HGSIM 0x1005ae58
void *__cdecl Sim_1005ae58(void *a, char *b)
{
    SIM_CALL_1010d180(b);
    return SIM_CALL_1005ae79(a, b);
}

// FUNCTION: HGSIM 0x1005502a
void __cdecl Sim_1005502a(void)
{
    SIM_CALL_100548aa();
    SIM_CALL_10054a79();
    ((int(__cdecl *)(void))(uintptr_t)U32(SIM_PTR(u32a, 0x1015fd20)[G32(0x1015fdf4)], 0x1c))();
}
