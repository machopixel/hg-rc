#include "w8v.h"
#include "../util/access.h"
#include "../util/w3o.h"

/*
 * Floating point: the x87 code runs with the control word 0x27f (53-bit precision), so every sum, product and
 * quotient below is rounded to double as there, and to float where the original stores a float: bit-exact.
 * fabs and sqrt (the C library's, called through their addresses) give results that are already doubles.
 */

/*
 * The rotation matrix (3 x 3 floats, rows of 3, then three zero floats) of the quaternion q = (x, y, z, w), scaled
 * by 2 / |q|^2 (0 when |q|^2 is below 1e-7).
 */
// FUNCTION: HGSIM 0x10060b76
void __cdecl Quat_ToMatrix(const float *q, float *m)
{
    double n;
    float nf, s, x, y, z, wx, wy, wz, xx, xy, xz, yy, yz, zz;

    if (q == 0)
        W8V_ASSERT(SIM_PTR(char, 0x10144d70), SIM_PTR(char, 0x10144d50), 0x22d);
    n = (double)q[0] * q[0] + (double)q[1] * q[1] + (double)q[2] * q[2] + (double)q[3] * q[3];
    nf = (float)n;
    if (!(W8V_FABS(n) >= (double)GFL(0x1012b510)))
        s = 0.0f;
    else
        s = (float)((double)GFL(0x1012b550) / (double)nf);
    x = (float)((double)q[0] * s);
    y = (float)((double)q[1] * s);
    z = (float)((double)q[2] * s);
    wx = (float)((double)q[3] * x);
    wy = (float)((double)q[3] * y);
    wz = (float)((double)q[3] * z);
    xx = (float)((double)q[0] * x);
    xy = (float)((double)q[0] * y);
    xz = (float)((double)q[0] * z);
    yy = (float)((double)q[1] * y);
    yz = (float)((double)q[1] * z);
    zz = (float)((double)q[2] * z);
    m[0] = (float)((double)GFL(0x1012b554) - ((double)yy + zz));
    m[3] = (float)((double)xy + wz);
    m[6] = (float)((double)xz - wy);
    m[1] = (float)((double)xy - wz);
    m[4] = (float)((double)GFL(0x1012b554) - ((double)xx + zz));
    m[7] = (float)((double)yz + wx);
    m[2] = (float)((double)xz + wy);
    m[5] = (float)((double)yz - wx);
    m[8] = (float)((double)GFL(0x1012b554) - ((double)xx + yy));
    U32(m, 0x2c) = 0;
    U32(m, 0x28) = 0;
    U32(m, 0x24) = 0;
}

/* Element (row, column) of a 3 x 3 float matrix. */
#define M(r, c) FL(m, (r) * 0xc + (c) * 4)

/* q[0], q[1] or q[2] for an index 0, 1 or anything else, like the original's switches. */
static float *Slot(float *q, int32_t i)
{
    return i == 0 ? &q[0] : i == 1 ? &q[1] : &q[2];
}

/*
 * The quaternion (x, y, z, w) of a rotation matrix. NOTE: the original first copies the element m[2][2] over
 * m[1][1] and m[1][1] over m[0][0] (so it writes to the matrix and uses m[2][2] as the "trace"); kept as it is.
 */
// FUNCTION: HGSIM 0x10060d2e
void __cdecl Matrix_ToQuat(float *m, float *q)
{
    const u32a *next = SIM_PTR(u32a, 0x10144cd8);
    double r;
    float s;
    int32_t i, j, k;

    if (m == 0)
        W8V_ASSERT(SIM_PTR(char, 0x10144d94), SIM_PTR(char, 0x10144d74), 0x254);
    U32(m, 0x10) = U32(m, 0x20);
    U32(m, 0) = U32(m, 0x10);
    if (FL(m, 0) > GFL(0x1012b510)) {
        r = W8V_SQRT((double)FL(m, 0) + GF64(0x1012b538));
        s = (float)r;
        q[3] = (float)(r * (double)GFL(0x1012b558));
        q[0] = (float)(((double)FL(m, 0x14) - FL(m, 0x1c)) * s);
        q[1] = (float)(((double)FL(m, 0x18) - FL(m, 8)) * s);
        q[2] = (float)(((double)FL(m, 4) - FL(m, 0xc)) * s);
        return;
    }
    i = 0;
    if (FL(m, 0x10) > FL(m, 0))
        i = 1;
    if (FL(m, 0x20) > M(i, i))
        i = 2;
    j = (int32_t)next[i];
    k = (int32_t)next[j];
    s = (float)W8V_SQRT((double)M(i, i) - ((double)M(j, j) + M(k, k)) + GF64(0x1012b538));
    if (i < 0 || i >= 3)
        W8V_ASSERT(SIM_PTR(char, 0x10144db8), SIM_PTR(char, 0x10144d98), 0x26a);
    if (j < 0 || j >= 3)
        W8V_ASSERT(SIM_PTR(char, 0x10144dec), SIM_PTR(char, 0x10144dcc), 0x26b);
    if (k < 0 || k >= 3)
        W8V_ASSERT(SIM_PTR(char, 0x10144e20), SIM_PTR(char, 0x10144e00), 0x26c);
    *Slot(q, i) = (float)((double)s * GFL(0x1012b558));
    s = (float)((double)GFL(0x1012b558) / s);
    q[3] = (float)(((double)M(j, k) - M(k, j)) * s);
    *Slot(q, j) = (float)(((double)M(i, j) - M(j, i)) * s);
    *Slot(q, k) = (float)(((double)M(i, k) + M(k, i)) * s);
}
