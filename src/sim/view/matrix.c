#include "view.h"

// FUNCTION: HGSIM 0x1006007d
void __cdecl Mat3_Transpose(const uint32_t *src, uint32_t *dst)
{
    uint32_t t;

    dst[0] = src[0];
    dst[4] = src[4];
    dst[8] = src[8];
    t = src[1];
    dst[1] = src[3];
    dst[3] = t;
    t = src[6];
    dst[6] = src[2];
    dst[2] = t;
    t = src[7];
    dst[7] = src[5];
    dst[5] = t;
}

// FUNCTION: HGSIM 0x1006018d
void __cdecl Mat3x4_SetIdentity(uint32_t *m)
{
    m[8] = 0x3f800000; /* 1.0f */
    m[4] = 0x3f800000;
    m[0] = 0x3f800000;
    m[9] = 0;
    m[6] = 0;
    m[3] = 0;
    m[10] = 0;
    m[7] = 0;
    m[1] = 0;
    m[11] = 0;
    m[5] = 0;
    m[2] = 0;
}
