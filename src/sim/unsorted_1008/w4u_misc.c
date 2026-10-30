#include "w4u.h"
#include "../core/core.h"
#include "../simmem.h"
#include "../unsorted_1001/w4u.h"
#include "../util/access.h"

// FUNCTION: HGSIM 0x1008b900
void __cdecl Sim_1008b900(void)
{
    if (G32(0x1015ecd0) == 0) {
        Timer_Pause(0x80, 0);
        Sim_1001f4d3();
    }
}
