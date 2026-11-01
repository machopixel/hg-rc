/*
 * Round 32: stand-ins for 1.2's particle spawns, for the specs of 1.0 functions that call them only in 1.2 mode
 * (W32_PARTICLES(), off in those specs) or stub them (spec_w32_debris.py). They never run.
 */
#include "../../src/sim/fx/w32.h"

void __cdecl Sim12_1007f8a9(float x, float y, float z, float dx, float dy, float dz)
{
    (void)x, (void)y, (void)z, (void)dx, (void)dy, (void)dz;
}

#define W32S_SIX(name) \
    void __cdecl name(float x, float y, float z, float dx, float dy, float dz) \
    { \
        (void)x, (void)y, (void)z, (void)dx, (void)dy, (void)dz; \
    }
W32S_SIX(Sim12_1007f19f)
W32S_SIX(Sim12_100805dc)
W32S_SIX(Sim12_10082556)
W32S_SIX(Sim12_10084362)
W32S_SIX(Sim12_1007e25c)
W32S_SIX(Sim12_10086007)

void __cdecl Sim12_1007c1a1(float x, float y, float z, float dx, float dy, float dz, int32_t strength)
{
    (void)x, (void)y, (void)z, (void)dx, (void)dy, (void)dz, (void)strength;
}

void __cdecl Sim12_1007d342(float x, float y, float z, float dx, float dy, float dz, int32_t kind, int32_t ground)
{
    (void)x, (void)y, (void)z, (void)dx, (void)dy, (void)dz, (void)kind, (void)ground;
}

void __cdecl Sim12_1008514d(float x, float y, float z, float dx, float dy, float dz, uint32_t unused, int32_t kind)
{
    (void)x, (void)y, (void)z, (void)dx, (void)dy, (void)dz, (void)unused, (void)kind;
}

void __cdecl Sim12_1007a0a6(uint32_t arg, int unused, uint32_t param)
{
    (void)arg, (void)unused, (void)param;
}

void __cdecl Sim12_1007acf3(uint32_t arg, int unused, uint32_t param, uint32_t param2)
{
    (void)arg, (void)unused, (void)param, (void)param2;
}

void __cdecl Sim12_1007b956(uint32_t arg, const uint8_t *shot, uint32_t param)
{
    (void)arg, (void)shot, (void)param;
}
