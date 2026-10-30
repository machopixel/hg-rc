#include "w3q.h"
#include "../render/w2n.h"
#include "../util/w3q.h"

// FUNCTION: HGSIM 0x100219a0
int __cdecl Sim_100219a0(int unused)
{
    double d = (double)GS32(0x1013b98c) / (double)GF32(0x1011120c);

    d = d * (double)GF32(0x10111210);
    d = d + (double)GF32(0x10111214);
    return Render_Ftol(d);
}
