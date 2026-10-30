#include "w3t.h"
#include "../util/w3t.h"
#include "../render/w2n.h"

/* 16.16 multiply, rounded: the 64-bit product shifted right 16 plus the last bit shifted out (shrd / adc). */
static int32_t FixMul(int32_t a, int32_t b)
{
    int64_t p = (int64_t)a * b;

    return (int32_t)((uint32_t)(p >> 16) + (uint32_t)((p >> 15) & 1));
}

// FUNCTION: HGSIM 0x1008ce40
int32_t *__cdecl Sim_1008ce40(int32_t *rect, float v)
{
    float scale = (float)((double)v / (double)0.833333313f);

    rect[1] = FixMul(rect[1], 0x13f);
    rect[2] = FixMul(rect[2], 0xc7);
    rect[2] = Render_Ftol((double)rect[2] * (double)scale);
    rect[3] = FixMul(rect[3], 0x13f);
    rect[4] = FixMul(rect[4], 0xc7);
    rect[4] = Render_Ftol((double)rect[4] * (double)scale);
    return rect;
}

// FUNCTION: HGSIM 0x1008e915
int __cdecl Sim_1008e915(const int32_t *rect, int x, int y)
{
    int32_t width = (int32_t)((uint32_t)rect[3] - (uint32_t)rect[1] + 1u);
    int32_t height = (int32_t)((uint32_t)rect[4] - (uint32_t)rect[2] + 1u);
    int32_t halfW = width >> 1;
    int32_t halfH = height >> 1;
    uint32_t limit = (uint32_t)(width / 2 - 1);
    int32_t dx = (int32_t)((uint32_t)x - (uint32_t)halfW);
    int32_t dy = Render_Ftol((double)(int32_t)((uint32_t)y - (uint32_t)halfH) / (double)FL32(G32(0x1014cae0), 0x44));

    limit = limit * limit;
    return (int32_t)((uint32_t)dx * (uint32_t)dx + (uint32_t)dy * (uint32_t)dy) <= (int32_t)limit;
}
