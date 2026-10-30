#include "w11.h"
#include "../simmem.h"
#include "../util/access.h"
#include "../util/w3o.h"
#include "../view/view.h"
#include "../view/w3r.h"
#include "../unsorted_1006/w5y.h"

#define W11_NODE(i) (SIM_PTR(uint8_t, 0x101a1180) + (uint32_t)(i) * 0x7cu)

typedef union {
    float f;
    uint32_t u;
} W11Bits;

/* one axis of the box: the size b must not be negative (min moved by it), then b = min + size */
static void W11_Axis(uint8_t *n, int a, int b)
{
    if ((U32(n, b) & 0x80000000u) != 0) {
        FL(n, a) = (float)((double)FL(n, a) + (double)FL(n, b));
        FL(n, b) = -FL(n, b);
    }
    FL(n, b) = (float)((double)FL(n, b) + (double)FL(n, a));
}

static uint32_t W11_Neg(uint8_t *n, int off)
{
    W11Bits v;

    v.f = -FL(n, off);
    return v.u;
}

// FUNCTION: HGSIM 0x100b3b16
void __cdecl Sim_100b3b16(const int32_t *part)
{
    uint32_t scale[12];
    float a[12], b[12];
    uint8_t *n, *p;
    float *mat;
    int i;

    if (G32(0x1017d224) == 0) {
        G32(0x101a1168) = 0;
        for (i = 0; i < GS32(0x101c42a4); i++)
            U32(BYTES(G32(0x1017d204)), (uint32_t)i * 4) = 0xffffffffu;
        G32(0x101c4220 + G32(0x1017d224) * 4) = 0xffffffffu;
    }
    if (GS32(0x1017d220) >= 0x20 || GS32(0x1017d220) < 0)
        return;
    G32(0x1017d224)++;
    G32(0x101c4220 + G32(0x1017d224) * 4) = G32(0x1017d220);
    n = W11_NODE(G32(0x1017d220));
    for (i = 0; i < 6; i++)
        FL(n, 4 * i) = (float)part[2 + i];
    U32(n, 0x78) = G32(0x101c421c + G32(0x1017d224) * 4);
    for (i = 0; i < 9; i++)
        U32(n, 0x24 + 4 * i) = G32(0x10150800 + 4 * (uint32_t)i);
    W11_Axis(n, 0, 0xc);
    W11_Axis(n, 4, 0x10);
    W11_Axis(n, 8, 0x14);
    FL(n, 0x18) = (float)(((double)FL(n, 0xc) + (double)FL(n, 0)) / (double)*SIM_PTR(const float, 0x1012bab8));
    FL(n, 0x1c) = (float)(((double)FL(n, 0x10) + (double)FL(n, 4)) / (double)*SIM_PTR(const float, 0x1012bab8));
    FL(n, 0x20) = (float)(((double)FL(n, 0x14) + (double)FL(n, 8)) / (double)*SIM_PTR(const float, 0x1012bab8));
    mat = (float *)(n + 0x48);

    /* a scale matrix from +0x24.. that the original builds and never uses */
    Mat3x4_SetIdentity(scale);
    scale[0] = U32(n, 0x24);
    scale[4] = U32(n, 0x28);
    scale[8] = U32(n, 0x2c);

    Sim_10060656((uint32_t)(uintptr_t)a, 0, 0, 0, W11_Neg(n, 0x18), W11_Neg(n, 0x1c), W11_Neg(n, 0x20));
    Sim_10060656((uint32_t)(uintptr_t)b, U32(n, 0x30), U32(n, 0x34), U32(n, 0x38), 0, 0, 0);
    Mat3x4_Multiply(b, a, mat);
    Sim_10060656((uint32_t)(uintptr_t)a, 0, 0, 0, U32(n, 0x3c), U32(n, 0x40), U32(n, 0x44));
    Mat3x4_Multiply(a, mat, mat);
    Sim_10060656((uint32_t)(uintptr_t)a, 0, 0, 0, U32(n, 0x18), U32(n, 0x1c), U32(n, 0x20));
    Mat3x4_Multiply(a, mat, mat);
    for (p = n; S32(p, 0x78) != -1;) {
        p = W11_NODE(U32(p, 0x78));
        Mat3x4_Multiply((const float *)(p + 0x48), mat, mat);
    }
    for (i = 0; i < 9; i++)
        G32(0x10150800 + 4 * (uint32_t)i) = G32(0x10150828 + 4 * (uint32_t)i);
    G32(0x101507f8) = G32(0x1017d220);
    G32(0x1017d220)++;
}
