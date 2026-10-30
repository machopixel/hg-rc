#include "w6i.h"
#include "../simmem.h"
#include "../util/access.h"
#include "../unsorted_1010/w2i.h"

static uint8_t W6i_FoldAscii(uint8_t c)
{
    return (uint8_t)(c - 0x41) < 0x1a ? (uint8_t)(c + 0x20) : c;
}

// FUNCTION: HGSIM 0x100ff1e0
int __cdecl Str_CompareNoCase(const char *a, const char *b)
{
    const uint8_t *s1 = (const uint8_t *)a;
    const uint8_t *s2 = (const uint8_t *)b;

    if (G32(0x1017e950) == 0) {
        for (;;) {
            uint8_t cb = *s2++;
            uint8_t ca = *s1++;

            if (ca != cb) {
                cb = W6i_FoldAscii(cb);
                ca = W6i_FoldAscii(ca);
                if (ca != cb)
                    return ca < cb ? -1 : 1;
            }
            if (cb == 0)
                return 0;
        }
    } else {
        uint32_t vb = 0xff; /* b's character in the low byte of the last fold result */
        uint32_t va = 0;    /* the same for a */

        for (;;) {
            if ((uint8_t)vb == 0)
                return (int)vb;
            vb = (vb & 0xffffff00u) | *s2++;
            va = (va & 0xffffff00u) | *s1++;
            if ((uint8_t)vb == (uint8_t)va)
                continue;
            va = W6I_FN_100FF280(va);
            vb = W6I_FN_100FF280(vb);
            if ((uint8_t)va == (uint8_t)vb)
                continue;
            return (uint8_t)va < (uint8_t)vb ? -1 : 1;
        }
    }
}

// FUNCTION: HGSIM 0x100ff4e0
char *__cdecl Sim_100ff4e0(void *value)
{
    int32_t *tm = W6I_FN_101071E0(value);

    if (tm == 0)
        return 0;
    return Time_FormatTm(tm);
}

// FUNCTION: HGSIM 0x100ffde0
char *__cdecl Text_IntToRadix(int value, char *buf, int radix)
{
    if (radix == 10 && value < 0)
        W6I_FN_100FFE20((uint32_t)value, buf, 10, 1);
    else
        W6I_FN_100FFE20((uint32_t)value, buf, (uint32_t)radix, 0);
    return buf;
}
