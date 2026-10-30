#include "w7r.h"
#include "w2i.h"
#include "w2j.h"
#include "w6j.h"

typedef union {
    double d;
    uint32_t w[2];
} W7rDoubleBits;

// FUNCTION: HGSIM 0x10103720
double __cdecl Math_ReportError(int type, uint32_t op, double arg1, double arg2, double retval, uint32_t cw)
{
    uint32_t name = Sim_10103800(op);

    (void)arg1;
    (void)arg2;
    W7R_FN_10103AD0(cw, 0xffff);
    if (name == 0 || Sim_101091f0() == 0)
        Sim_101037d0(type);
    return retval;
}

// FUNCTION: HGSIM 0x101039a0
double __cdecl Math_FrExp(double x, int32_t *expo)
{
    W7rDoubleBits u;
    uint32_t hi, lo;
    int32_t e;

    u.d = x;
    hi = u.w[1];
    lo = u.w[0];
    /* the original's x87 compare with 0.0 also takes the unordered (NaN) case as equal */
    if (((hi & 0x7fffffffu) == 0 && lo == 0) ||
        ((hi & 0x7ff00000u) == 0x7ff00000u && ((hi & 0xfffffu) != 0 || lo != 0))) {
        *expo = 0;
        return 0.0;
    }
    if ((hi & 0x7ff00000u) != 0) {
        *expo = (int32_t)((hi >> 20) & 0x7ffu) - 0x3fe;
        return Sim_10103870(x, 0);
    }
    /* denormal: shift the mantissa up until the hidden-bit position is set */
    e = -0x3fd;
    while ((hi & 0x00100000u) == 0) {
        hi = (hi << 1) | (lo >> 31);
        lo <<= 1;
        e--;
    }
    hi &= ~0x00100000u;
    if (u.w[1] & 0x80000000u)
        hi |= 0x80000000u;
    u.w[0] = lo;
    u.w[1] = hi;
    *expo = e;
    return Sim_10103870(u.d, 0);
}
