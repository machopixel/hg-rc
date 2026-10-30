#include "w5x.h"
#include "../unsorted_100f/w2e.h"
#include "../util/access.h"

// FUNCTION: HGSIM 0x10034777
void __cdecl Sim_10034777(const char *s)
{
    int length = (int)Str_Length(s);
    int i;

    for (i = 0; i < length; i += 0x50)
        SIM_CALL_1003462f(s + i);
}

// FUNCTION: HGSIM 0x100347b9
void __cdecl Sim_100347b9(void)
{
    G32(0x1015e500) = SIM_CALL_100fd850(GBYTES(0x10139e58), GBYTES(0x10139fb8));
}
