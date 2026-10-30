#include "w2i.h"
#include "../util/access.h"

/* 16.16 multiply, rounded: the 64-bit product shifted right 16 plus the last bit shifted out (shrd / adc). */
static int32_t FixMul(int32_t a, int32_t b)
{
    int64_t p = (int64_t)a * b;

    return (int32_t)((uint32_t)(p >> 16) + (uint32_t)((p >> 15) & 1));
}

// FUNCTION: HGSIM 0x10083495
void __cdecl Sim_10083495(void)
{
}

// FUNCTION: HGSIM 0x1008d181
int32_t *__cdecl Rect_ScaleAroundCenter(const int32_t *rect, int32_t *out, int32_t sx, int32_t sy)
{
    int32_t cx = ((int32_t)(U32(rect, 0xc) - U32(rect, 4) + 1u) >> 1) + S32(rect, 4);
    int32_t cy = ((int32_t)(U32(rect, 0x10) - U32(rect, 8) + 1u) >> 1) + S32(rect, 8);

    U32(out, 4) = U32(rect, 4) - (uint32_t)cx;
    U32(out, 8) = U32(rect, 8) - (uint32_t)cy;
    U32(out, 0xc) = U32(rect, 0xc) - (uint32_t)cx;
    U32(out, 0x10) = U32(rect, 0x10) - (uint32_t)cy;
    S32(out, 4) = FixMul(S32(out, 4), sx);
    S32(out, 8) = FixMul(S32(out, 8), sy);
    S32(out, 0xc) = FixMul(S32(out, 0xc), sx);
    S32(out, 0x10) = FixMul(S32(out, 0x10), sy);
    U32(out, 4) = U32(out, 4) + (uint32_t)cx;
    U32(out, 8) = U32(out, 8) + (uint32_t)cy;
    U32(out, 0xc) = U32(out, 0xc) + (uint32_t)cx;
    U32(out, 0x10) = U32(out, 0x10) + (uint32_t)cy;
    return out;
}
