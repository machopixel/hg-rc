#include "w5x.h"
#include "fns.h"
#include "w2l.h"
#include "../util/access.h"

// FUNCTION: HGSIM 0x1004c362
int __cdecl Sim_1004c362(uint32_t word)
{
    int root = (int)SIM_CALL_100fca04((double)(int32_t)Sim_1004c318(word));

    return (int)((uint32_t)root + U32(GBYTES(0x10124ec8u + (uint32_t)Sim_1004c30d((int)word) * 0x98u), 0));
}
