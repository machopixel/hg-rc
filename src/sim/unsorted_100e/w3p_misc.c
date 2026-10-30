#include "w3p.h"
#include "../util/w3p.h"

// FUNCTION: HGSIM 0x100e2756
void __cdecl Sim_100e2756(int a, int b, int c)
{
    GF32(0x1017e844) = (float)a;
    GF32(0x1017e848) = (float)b;
    GF32(0x1017e84c) = (float)c;
}
