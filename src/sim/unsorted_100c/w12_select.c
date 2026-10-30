#include "w12.h"
#include "../simmem.h"
#include "../util/access.h"
#include "../util/w3o.h"
#include "../view/w3r.h"
#include "../unsorted_100a/w5x.h"
#include "../unsorted_100b/fns.h"
#include "../unsorted_100b/w2f.h"

typedef union {
    float f;
    uint32_t u;
} W12Bits;

/* One offset of part, as float bits: 0 when it equals [0x1012bb18] (or is NaN: fcomp's equal flag), scale (its bits
 * copied, as the original moves them) when its bits are 1.0, else scale / it. */
static uint32_t W12_Offset(int16_t part, uint32_t table, uint32_t scale)
{
    uint32_t addr = table + (uint32_t)(int32_t)part * 8u;
    float v = GFL(addr);
    W12Bits s, q;

    if (!(v < GFL(0x1012bb18)) && !(v > GFL(0x1012bb18)))
        return 0;
    if (G32(addr) == 0x3f800000u)
        return scale;
    s.u = scale;
    q.f = (float)((double)s.f / (double)v);
    return q.u;
}

static float W12_Float(uint32_t bits)
{
    W12Bits b;

    b.u = bits;
    return b.f;
}

// FUNCTION: HGSIM 0x100c0158
void __cdecl Sim_100c0158(uint32_t sel, int16_t part, float *x, float *y, uint32_t *out, uint32_t scale)
{
    float zero = 0.0f;
    uint32_t index = sel & 0xff;
    uint32_t node = 0; /* start value for other kinds (DECISIONS.md) */

    if ((sel & 0xf00) == 0x200) {
        *out = U32(PTRTAB(index), 0x68);
        node = U32(PTRTAB(index), 0x48);
    } else if ((sel & 0xf00) == 0x400) {
        uint32_t obj = G32(0x109e9ba8 + index * 0x50);

        node = Sim_100b5386((int)obj);
        if (node == 0) {
            Sim_100b53f0((int)obj, (uint32_t *)x, out, (uint32_t *)y);
            *out = 0;
            *x = (float)((double)*x + (double)W12_Float(W12_Offset(part, 0x101516f8, scale)));
            *y = (float)((double)*y + (double)W12_Float(W12_Offset(part, 0x101516fc, scale)));
            return;
        }
    }
    *(uint32_t *)x = W12_Offset(part, 0x101516f8, scale);
    *(uint32_t *)y = W12_Offset(part, 0x101516fc, scale);
    Mat3x4_TransformPoint((const float *)Sim_100aa863((void *)(uintptr_t)node), x, &zero, y);
}