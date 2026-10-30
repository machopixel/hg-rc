#include "w8y.h"
#include "../util/access.h"
#include "../util/w3o.h"
#include "../unsorted_1008/w3o.h"
#include "w7o.h"

/*
 * Floating point as in util/w3o.h: double intermediates (the x87 runs with a 53-bit mantissa), float where the
 * original stores a float, truncation through Render_Ftol.
 */

// FUNCTION: HGSIM 0x100a648c
int __cdecl Sim_100a648c(float px, float py, float pz, float dx, float dy, float dz)
{
    float value;
    float ex, ey, ez;
    float dot;
    int16_t v;

    if (G32(0x109afdcc) != 0) {
        value = (float)((double)dx * GFL(0x109b16c4) + (double)dy * GFL(0x109b16c8) + (double)dz * GFL(0x109b16c0));
    } else {
        ex = (float)((double)GFL(0x109b16c4) - px);
        ey = (float)((double)GFL(0x109b16c8) - py);
        ez = (float)((double)GFL(0x109b16c0) - pz);
        if (!(W8Y_CALL_100fcb70(ex) >= GFL(0x1012ba74)) && !(W8Y_CALL_100fcb70(ey) >= GFL(0x1012ba74))
            && !(W8Y_CALL_100fcb70(ez) >= GFL(0x1012ba74)))
            return 0x7f;
        dot = (float)((double)dx * ex + (double)dy * ey + (double)dz * ez);
        value = (float)((double)dot / Sim_10084a94(ex, ey, ez));
    }
    v = (int16_t)Render_Ftol((double)value * GFL(0x1012ba7c));
    if (v < -0x7f)
        v = -0x7f;
    else if (v > 0x7f)
        v = 0x7f;
    return v;
}

/* fld / fchs / fstp of a float: the sign flips, and a signalling NaN comes out quiet as it does through the x87. */
static uint32_t neg_bits(uint32_t u)
{
    uint32_t r = u ^ 0x80000000u;

    if ((u & 0x7f800000u) == 0x7f800000u && (u & 0x007fffffu) != 0)
        r |= 0x00400000u;
    return r;
}

// FUNCTION: HGSIM 0x100aa016
void __cdecl Sim_100aa016(void *node, uint32_t a, uint32_t b, uint32_t c, uint32_t mode, uint32_t x, uint32_t y,
                          uint32_t z)
{
    float m[12];

    U32(node, 0x94) |= 1;
    W8Y_CALL_10060220(m, 0, 0, 0, neg_bits(x), neg_bits(y), neg_bits(z), mode);
    Sim_100a9f6e(node, m);
    W8Y_CALL_10060220(m, a, b, c, 0, 0, 0, mode);
    Sim_100a9f6e(node, m);
    W8Y_CALL_10060220(m, 0, 0, 0, x, y, z, mode);
    Sim_100a9f6e(node, m);
}
