#include "view.h"
#include "w2d.h"
#include "../util/access.h"

/* Signed (num << 16) / den truncated toward zero, as the original's cdq/shld/idiv does. */
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

// FUNCTION: HGSIM 0x1008a37f
void __cdecl Sim_1008a37f(void *node)
{
    if (node == 0)
        return;
    if ((U16(node, 0) & 0x800) != 0)
        return;
    ObjList_Unlink(node);
    U16(node, 0) = (uint16_t)(U16(node, 0) | 0x800);
    if (G32(0x1017c8cc) == 0)
        return;
    if (U32(node, 0x10) != 0)
        U32(U32(node, 0x10), 0xc) = U32(node, 0xc);
    if (U32(node, 0xc) != 0)
        U32(U32(node, 0xc), 0x10) = U32(node, 0x10);
    U32(node, 0xc) = 0;
    U32(node, 0x10) = 0;
}

// FUNCTION: HGSIM 0x1008f747
int32_t *__cdecl Sim_1008f747(int flip, const uint32_t *pair)
{
    uint32_t n = pair[0];
    uint32_t o = pair[1];
    uint32_t out = 0;
    uint32_t state = U32(n, 0);

    if (state == 1) {
        uint32_t start, end;

        out = U32(o, 0xc);
        if (flip == 0) {
            start = U32(o, 4);
            end = U32(o, 8);
        } else {
            start = U32(o, 8);
            end = U32(o, 4);
        }
        if (U32(n, 4) == 0) {
            CopyRect(out, start);
            U32(n, 8) = 0;
        } else {
            int32_t t;

            U32(n, 8) = U32(n, 8) + G32(0x1015dc80);
            t = DivShifted16(S32(n, 8), S32(o, 0));
            if (t >= 0x10000) {
                CopyRect(out, end);
                U32(n, 8) = 0;
                state = 0;
            } else {
                Rect_Interpolate((const int32_t *)(uintptr_t)start, (const int32_t *)(uintptr_t)end,
                                 (int32_t *)(uintptr_t)out, t, 0);
            }
        }
    }
    U32(n, 4) = U32(n, 0);
    U32(n, 0) = state;
    return (int32_t *)(uintptr_t)out;
}

// FUNCTION: HGSIM 0x100d25eb
void __cdecl Sim_100d25eb(void *dst, const uint32_t *src)
{
    Mat3_Transpose(src, (uint32_t *)(BYTES(dst) + 0x54));
    U32(dst, 0x78) = U32(src, 0x24);
    U32(dst, 0x7c) = U32(src, 0x28);
    U32(dst, 0x80) = U32(src, 0x2c);
}

// FUNCTION: HGSIM 0x100d262a
void __cdecl Sim_100d262a(const void *src, uint32_t *dst2)
{
    Mat3_Transpose((const uint32_t *)(BYTES(src) + 0x54), dst2);
    U32(dst2, 0x24) = U32(src, 0x78);
    U32(dst2, 0x28) = U32(src, 0x7c);
    U32(dst2, 0x2c) = U32(src, 0x80);
}
