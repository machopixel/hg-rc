#include "w8A.h"
#include "w7r.h"
#include "w2f.h"
#include "../unsorted_100f/w8A.h"
#include "../unsorted_100f/w7r.h"
#include "../unsorted_100f/w6h.h"
#include "../util/access.h"
#include "../crt/hg_format.h"

#define ADDR(p) ((uint32_t)(uintptr_t)(p))

// FUNCTION: HGSIM 0x101055e0
uint32_t __cdecl Crt_StrNLen(const char *s, uint32_t max)
{
    uint32_t i = 0;

    if (max != 0) {
        while (s[i] != 0) {
            i++;
            if (i == max)
                break;
        }
    }
    return s[i] == 0 ? i : max;
}

// FUNCTION: HGSIM 0x1010c6a0
uint32_t __cdecl Crt_WcsNCount(const uint16_t *s, uint32_t max)
{
    const uint16_t *p = s;
    uint32_t n = max;

    if (n == 0)
        return max;
    while (*p != 0) {
        p++;
        if (--n == 0)
            return max;
    }
    return (uint32_t)((int32_t)((const char *)p - (const char *)s) >> 1) + 1u;
}

// FUNCTION: HGSIM 0x101006c0
int32_t __cdecl Crt_VSPrintf(char *buf, const char *fmt, void *args)
{
    /* C runtime replacement, formatting: SDL3's formatter (crt/hg_format.h), unbounded as the original */
    return hg_vformat(buf, 0x7fffffff, fmt, (va_list)args);
}

// FUNCTION: HGSIM 0x10100a90
void __cdecl Crt_ForceDecPoint(char *buf)
{
    char *p = buf;
    char c, next, stored;

    if (Crt_ToLower(*p) != 'e') {
        int isdigit;

        do {
            p++;
            if (GS32(0x1015965c) > 1)
                isdigit = Crt_IsCType(*p, 4);
            else
                isdigit = *(const uint8_t *)(uintptr_t)(G32(0x10159450) + 2u * (uint32_t)(int32_t)*p) & 4;
        } while (isdigit != 0);
    }
    c = *p;
    *p++ = (char)GBYTES(0x10159660)[0];
    do {
        next = *p;
        *p = c;
        stored = *p;
        p++;
        c = next;
    } while (stored != 0);
}

// FUNCTION: HGSIM 0x10100f00
char *__cdecl Sim_10100f00(const uint32_t *value, char *buf, int ndig)
{
    char *r;

    GBYTES(0x1017e908)[0] = 1;
    r = Fmt_DoubleF(value, buf, ndig);
    GBYTES(0x1017e908)[0] = 0;
    return r;
}

// FUNCTION: HGSIM 0x10108da0
int __cdecl Sim_10108da0(uint32_t *out, const char *text)
{
    uint16_t rec[6];
    const char *end;

    W8A_FN_1010b7c0(rec, &end, text, 0, 0, 0, 0);
    return Sim_10108ce0(rec, out);
}
