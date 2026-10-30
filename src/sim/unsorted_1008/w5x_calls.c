#include "w5x.h"
#include "../unsorted_1000/fns.h"
#include "../unsorted_1000/w5x.h"
#include "../util/access.h"

// FUNCTION: HGSIM 0x1008b757
void __cdecl Sim_1008b757(void)
{
    Sim_1000e983((const char *)GBYTES(0x1014b950), 0, 0, Sim_1000edce());
}

// FUNCTION: HGSIM 0x1008b773
void __cdecl Sim_1008b773(void)
{
    Sim_1000e983((const char *)GBYTES(0x1014b95c), 0, 0, Sim_1000edce());
}
