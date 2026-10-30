#include "w3p.h"
#include "../render/w2n.h"
#include "../util/access.h"
#include "../util/w3p.h"

// FUNCTION: HGSIM 0x10047aa0
int __cdecl Sim_10047aa0(int32_t *p, int a, int b, double d)
{
    p[1] = a;
    p[2] = b;
    p[3] = Render_Ftol(d * GF64(0x101115d8));
    p[0] = (int32_t)G32(0x1015dc78);
    return p[3] > 0;
}

/*
 * The original divides the two integers on the x87 stack: the quotient is stored as a float, but the
 * comparisons that follow `fst` use the unrounded double that is still on the stack.
 */
// FUNCTION: HGSIM 0x1004bce5
void __cdecl Sim_1004bce5(int32_t *p, int a, int b, int c, int d)
{
    int32_t adjust = 0;
    int atTop = 0;
    double q;
    float r = (float)((double)c / (double)d);

    if ((double)r > (double)GF32(0x10126ec0))
        atTop = 1;
    else if ((double)r > GF64(0x10126ec8))
        adjust = -5;
    else if ((double)r > GF64(0x10126ed0))
        adjust = -5;
    else if ((double)r > GF64(0x10126ed8))
        adjust = -3;
    else if ((double)r > GF64(0x10126ee0))
        adjust = -2;
    else if ((double)r > (double)GF32(0x10126ee8))
        adjust = -1;

    q = (double)a / (double)b;
    r = (float)q;
    if (X87_EqualOrUnordered(q, GF64(0x10126ef0)))
        adjust = (int32_t)((uint32_t)adjust - 3u);
    else if (!X87_LessOrUnordered((double)r, GF64(0x10126ef8)))
        adjust = (int32_t)((uint32_t)adjust - 2u);
    else if (!X87_LessOrUnordered((double)r, GF64(0x10126f00)))
        adjust = (int32_t)((uint32_t)adjust - 1u);

    if (atTop != 0)
        p[0] = 1;
    else
        p[0] = (int32_t)((uint32_t)p[0] + (uint32_t)adjust);
}

// FUNCTION: HGSIM 0x1004be11
int __cdecl Sim_1004be11(int a, int b)
{
    double q = (double)a / (double)b;
    float r = (float)q;

    if (q > (double)GF32(0x10126ec0))
        return -8;
    if ((double)r > GF64(0x10126ec8))
        return -6;
    if ((double)r > GF64(0x10126ed0))
        return -5;
    if ((double)r > GF64(0x10126ed8))
        return -4;
    if ((double)r > GF64(0x10126ee0))
        return -3;
    if ((double)r > (double)GF32(0x10126ee8))
        return -2;
    return 0;
}
