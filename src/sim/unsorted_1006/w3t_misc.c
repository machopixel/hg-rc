#include "w3t.h"
#include "../util/w3t.h"

// FUNCTION: HGSIM 0x10068e55
float __cdecl Sim_10068e55(const float *p, int row, int column)
{
    if (p)
        return FL32(p, (uint32_t)(column + row * 2) * 4u);
    return 0.0f;
}
