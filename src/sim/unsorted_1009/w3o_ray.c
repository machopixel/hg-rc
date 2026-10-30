#include "w3o.h"
#include "../unsorted_1008/w3o.h"
#include "../util/w3o.h"

// FUNCTION: HGSIM 0x1009cbde
float __cdecl Sim_1009cbde(void *seg)
{
    if (U32(seg, 0x34) == 0)
        Sim_1009cbfe(seg);
    return FL(seg, 0x30);
}

// FUNCTION: HGSIM 0x1009cbfe
void __cdecl Sim_1009cbfe(void *seg)
{
    if (U32(seg, 0x34) == 0) {
        float length;

        FL(seg, 0x30) = (float)Sim_10084a94(FL(seg, 0x18), FL(seg, 0x1c), FL(seg, 0x20));
        length = FL(seg, 0x30);
        if (length > GFL(0x1012ba04)) {
            FL(seg, 0x24) = (float)((double)FL(seg, 0x18) / length);
            FL(seg, 0x28) = (float)((double)FL(seg, 0x1c) / length);
            FL(seg, 0x2c) = (float)((double)FL(seg, 0x20) / length);
            U32(seg, 0x34) = 1;
        }
    }
}

// FUNCTION: HGSIM 0x1009cd36
void __cdecl Sim_1009cd36(void *seg, float t)
{
    if (U32(seg, 0x34) == 0)
        Sim_1009cbfe(seg);
    if (U32(seg, 0x34) != 0) {
        FL(seg, 0xc) = (float)((double)t * FL(seg, 0x24) + FL(seg, 0));
        FL(seg, 0x10) = (float)((double)t * FL(seg, 0x28) + FL(seg, 4));
        FL(seg, 0x14) = (float)((double)t * FL(seg, 0x2c) + FL(seg, 8));
        FL(seg, 0x18) = (float)((double)FL(seg, 0xc) - FL(seg, 0));
        FL(seg, 0x1c) = (float)((double)FL(seg, 0x10) - FL(seg, 4));
        FL(seg, 0x20) = (float)((double)FL(seg, 0x14) - FL(seg, 8));
        U32(seg, 0x30) = Bits_OfFloat(t);
    }
}
