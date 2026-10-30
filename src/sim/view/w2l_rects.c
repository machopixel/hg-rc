#include "w2l.h"
#include "view.h"
#include "../util/access.h"

/* Signed (num << 16) / den truncated toward zero, as the original's cdq/shld/idiv does (it traps where this wraps). */
static int32_t DivShifted16(int32_t num, int32_t den)
{
    uint32_t nmag = num < 0 ? 0u - (uint32_t)num : (uint32_t)num;
    uint32_t dmag = den < 0 ? 0u - (uint32_t)den : (uint32_t)den;
    uint64_t n = (uint64_t)nmag << 16;
    uint64_t rem = 0, quot = 0;
    int i;

    if (dmag == 0)
        return 0;
    for (i = 47; i >= 0; i--) {
        rem = (rem << 1) | ((n >> i) & 1);
        if (rem >= dmag) {
            rem -= dmag;
            quot |= (uint64_t)1 << i;
        }
    }
    return (num < 0) != (den < 0) ? (int32_t)(0u - (uint32_t)quot) : (int32_t)(uint32_t)quot;
}

static void CopyRect(uint32_t dst, uint32_t src)
{
    int i;

    for (i = 0; i < 5; i++)
        U32(dst, i * 4) = U32(src, i * 4);
}

// FUNCTION: HGSIM 0x1008d07b
int32_t *__cdecl Sim_1008d07b(const int32_t *in, int32_t *out)
{
    int32_t x = in[0];
    int32_t y;

    out[0] = DivShifted16(x, 0x13f);
    y = in[1];
    out[1] = DivShifted16(y, 0xc7);
    return out;
}

// FUNCTION: HGSIM 0x1008f867
int32_t *__cdecl Sim_1008f867(int flip, const uint32_t *pair)
{
    uint32_t n = pair[0];
    uint32_t o = pair[1];
    uint32_t out = 0;
    uint32_t state = U32(n, 0);

    if (state == 1) {
        uint32_t a, b;
        int32_t skipA, skipB;

        out = U32(o, 0xc);
        if (flip == 0) {
            a = U32(o, 4);
            b = U32(o, 8);
            skipA = 1;
            skipB = 2;
        } else {
            a = U32(o, 8);
            b = U32(o, 4);
            skipA = 2;
            skipB = 1;
        }
        if (U32(n, 4) == 0) {
            CopyRect(out, a);
            U32(n, 8) = 0;
        } else {
            int32_t t;

            U32(n, 8) = U32(n, 8) + G32(0x1015dc80);
            t = DivShifted16(S32(n, 8), S32(o, 0));
            if (t >= 0x10000) {
                CopyRect(out, b);
                U32(n, 8) = 0;
                state = 0;
            } else if (t <= 0x8000) {
                CopyRect(out, a);
                /* t * 0x20000 >> 16 (imul, shrd, adc) */
                Rect_Interpolate((const int32_t *)(uintptr_t)a, (const int32_t *)(uintptr_t)b, (int32_t *)(uintptr_t)out,
                                 (int32_t)((uint32_t)t * 2u), skipA);
            } else {
                CopyRect(out, b);
                Rect_Interpolate((const int32_t *)(uintptr_t)a, (const int32_t *)(uintptr_t)b, (int32_t *)(uintptr_t)out,
                                 (int32_t)(((uint32_t)t - 0x8000u) * 2u), skipB);
            }
        }
    }
    U32(n, 4) = U32(n, 0);
    U32(n, 0) = state;
    return (int32_t *)(uintptr_t)out;
}
