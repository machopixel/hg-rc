#include "w3r.h"
#include "view.h"
#include "../util/w3r.h"
#include "../unsorted_1002/w2e.h"

// FUNCTION: HGSIM 0x1005fbb0
void __cdecl Mat3x4_TransformPoint(const float *m, float *x, float *y, float *z)
{
    float px = *x, py = *y, pz = *z;

    *x = (float)((double)m[0] * px + (double)m[1] * py + (double)m[2] * pz + (double)m[9]);
    *y = (float)((double)m[3] * px + (double)m[4] * py + (double)m[5] * pz + (double)m[10]);
    *z = (float)((double)m[6] * px + (double)m[7] * py + (double)m[8] * pz + (double)m[11]);
}

// FUNCTION: HGSIM 0x1005fcdc
void __cdecl Mat3_CrossColumn(float *m, int axis)
{
    float a, b, c, d, e, f; /* the two other columns: (a, b, c) and (d, e, f) */
    float r0, r1, r2;

    switch (axis) {
    case 0:
        a = m[1]; b = m[4]; c = m[7]; d = m[2]; e = m[5]; f = m[8];
        break;
    case 1:
        a = m[2]; b = m[5]; c = m[8]; d = m[0]; e = m[3]; f = m[6];
        break;
    case 2:
        a = m[0]; b = m[3]; c = m[6]; d = m[1]; e = m[4]; f = m[7];
        break;
    default:
        return;
    }
    r0 = (float)((double)b * f - (double)e * c);
    r1 = (float)((double)c * d - (double)f * a);
    r2 = (float)((double)a * e - (double)d * b);
    m[axis] = r0;
    m[axis + 3] = r1;
    m[axis + 6] = r2;
}

// FUNCTION: HGSIM 0x1005fe48
void __cdecl Mat3_Multiply(const float *a, const float *b, float *out)
{
    double x[9], y[9];
    int i, j;

    for (i = 0; i < 9; i++) {
        x[i] = a[i];
        y[i] = b[i];
    }
    for (i = 0; i < 3; i++)
        for (j = 0; j < 3; j++)
            out[i * 3 + j] = (float)(x[i * 3] * y[j] + x[i * 3 + 1] * y[3 + j] + x[i * 3 + 2] * y[6 + j]);
}

// FUNCTION: HGSIM 0x10060008
void __cdecl Mat3x4_Multiply(const float *a, const float *b, float *out)
{
    float t[12];
    float p0, p1, p2;

    Mat3_Multiply(a, b, t);
    p0 = b[9];
    p1 = b[10];
    p2 = b[11];
    Mat3x4_TransformPoint(a, &p0, &p1, &p2);
    t[9] = p0;
    t[10] = p1;
    t[11] = p2;
    Sim_10028571(out, t, 0x30);
}

// FUNCTION: HGSIM 0x10060101
void __cdecl Mat3x4_InvertRigid(const float *m, float *out)
{
    float n0 = -m[9], n1 = -m[10], n2 = -m[11];

    Mat3_Transpose((const uint32_t *)m, (uint32_t *)out);
    U32(out, 0x24) = 0;
    U32(out, 0x28) = 0;
    U32(out, 0x2c) = 0;
    Mat3x4_TransformPoint(out, &n0, &n1, &n2);
    out[9] = n0;
    out[10] = n1;
    out[11] = n2;
}

// FUNCTION: HGSIM 0x100b403d
void __cdecl Sim_100b403d(float *p)
{
    int32_t index = GS32(0x101507f8);

    if (index != -1)
        Mat3x4_TransformPoint(SIM_PTR(float, 0x101a11c8u + (uint32_t)index * 0x7cu), p, p + 1, p + 2);
}
