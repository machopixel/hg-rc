/*
 * Test entries for the checker only (spec_w3p.py): the original functions that return a float in st(0)
 * are compared through the bit pattern, so each is wrapped here into a function that returns the bits in eax
 * (the spec wraps the original the same way with a few instructions of machine code).
 */
#include "sim/render/w3p.h"
#include "sim/unsorted_1006/w3p.h"
#include "sim/unsorted_100c/w3p.h"

static unsigned Bits(float f)
{
    union {
        float f;
        unsigned u;
    } v;

    v.f = f;
    return v.u;
}

unsigned __cdecl Test_Clamp(float a, float b, float c) { return Bits(Math_ClampFloat_100686ea(a, b, c)); }
unsigned __cdecl Test_ClampSym(float a, float b) { return Bits(Math_ClampSymmetric_100c351c(a, b)); }
unsigned __cdecl Test_100c05cb(const void *obj) { return Bits(Sim_100c05cb(obj)); }
unsigned __cdecl Test_Fog(float z, float arg) { return Bits(Render_FogFactor_10058ac2(z, arg)); }
