#include "w5x.h"
#include "../util/util.h"

// FUNCTION: HGSIM 0x100b5b76
void __cdecl Sim_100b5b76(void *obj)
{
    void *callback = 0;

    if (obj == 0)
        return;
    if (Sim_100a9b71(obj) != 0)
        callback = SIM_PTR(uint8_t, 0x100010f5u);
    SIM_CALL_100aab03(obj, callback);
}
