#include "w7q.h"
#include "w6h.h"
#include "../util/access.h"

/* Class test of the parsers: the table word, or Crt_IsCType when the dword 0x1015965c is above 1. */
static int W7q_Class(uint32_t c, int mask)
{
    if (GS32(0x1015965c) > 1)
        return Crt_IsCType((int)c, mask);
    return U16(G32(0x10159450), c * 2u) & mask;
}

// FUNCTION: HGSIM 0x100fd1c0
int64_t __cdecl Crt_AToI64(const char *s)
{
    const uint8_t *p = (const uint8_t *)s;
    uint32_t c;
    uint32_t sign;
    uint64_t total = 0;

    while (W7q_Class(*p, 8) != 0)
        p++;
    c = *p++;
    sign = c;
    if (c == '-' || c == '+')
        c = *p++;
    while (W7q_Class(c, 4) != 0) {
        total = total * 10u + (uint64_t)(int64_t)((int32_t)c - 0x30);
        c = *p++;
    }
    if (sign == '-')
        return (int64_t)(0u - total);
    return (int64_t)total;
}

// FUNCTION: HGSIM 0x100fd2b0
uint32_t __cdecl Crt_StrToXL(const char *s, const char **end, int base, uint32_t flags)
{
    const uint8_t *p = (const uint8_t *)s;
    uint32_t number = 0;
    uint32_t limit;
    uint32_t digit;
    uint8_t c;

    c = *p++;
    while (W7q_Class(c, 8) != 0)
        c = *p++;
    if (c == '-') {
        flags |= 2;
        c = *p++;
    } else if (c == '+') {
        c = *p++;
    }
    if (base < 0 || base == 1 || base > 36) {
        if (end != 0)
            *end = s;
        return 0;
    }
    if (base == 0) {
        if (c != '0')
            base = 10;
        else if (*p == 'x' || *p == 'X')
            base = 16;
        else
            base = 8;
    }
    if (base == 16 && c == '0' && (*p == 'x' || *p == 'X')) {
        c = p[1];
        p += 2;
    }
    limit = 0xffffffffu / (uint32_t)base;
    for (;;) {
        if (W7q_Class(c, 4) != 0)
            digit = (uint32_t)((int32_t)(int8_t)c - 0x30);
        else if (W7q_Class(c, 0x103) != 0)
            digit = (uint32_t)Crt_ToUpper((int8_t)c) - 0x37u;
        else
            break;
        if (digit >= (uint32_t)base)
            break;
        flags |= 8;
        if (number < limit || (number == limit && digit <= 0xffffffffu % (uint32_t)base))
            number = number * (uint32_t)base + digit;
        else
            flags |= 4;
        c = *p++;
    }
    p--;
    if ((flags & 8) == 0) {
        if (end != 0)
            p = (const uint8_t *)s;
        number = 0;
    } else if ((flags & 4) != 0 ||
               ((flags & 1) == 0 && (((flags & 2) != 0 && number > 0x80000000u) ||
                                     ((flags & 2) == 0 && number > 0x7fffffffu)))) {
        G32(0x1017e888) = 0x22;
        if ((flags & 1) != 0)
            number = 0xffffffffu;
        else
            number = (flags & 2) != 0 ? 0x80000000u : 0x7fffffffu;
    }
    if (end != 0)
        *end = (const char *)p;
    if ((flags & 2) != 0)
        number = 0u - number;
    return number;
}

// FUNCTION: HGSIM 0x100fdb80
int __cdecl Crt_ToUpper(int c)
{
    uint8_t in[4];
    uint8_t out[4];
    int n;
    int got;

    if (G32(0x1017e950) == 0) {
        if (c >= 'a' && c <= 'z')
            return c - 0x20;
        return c;
    }
    if (c < 0x100) {
        int lower;

        if (GS32(0x1015965c) > 1)
            lower = Crt_IsCType(c, 2);
        else
            lower = U8(G32(0x10159450), (uint32_t)c * 2u) & 2;
        if (lower == 0)
            return c;
    }
    /* the original builds the source bytes in the stack slot of its argument, so the bytes it does not set
       keep the argument's bytes */
    in[0] = (uint8_t)c;
    in[1] = (uint8_t)((uint32_t)c >> 8);
    in[2] = (uint8_t)((uint32_t)c >> 16);
    in[3] = (uint8_t)((uint32_t)c >> 24);
    if ((U8(G32(0x10159450), (((uint32_t)c >> 8) & 0xffu) * 2u + 1u) & 0x80) != 0) {
        in[0] = (uint8_t)((uint32_t)c >> 8);
        in[1] = (uint8_t)c;
        in[2] = 0;
        n = 2;
    } else {
        in[0] = (uint8_t)c;
        in[1] = 0;
        n = 1;
    }
    got = W7Q_MAP_STRING(G32(0x1017e950), 0x200, in, n, out, 3, 0);
    if (got == 0)
        return c;
    if (got == 1)
        return out[0];
    return ((int)out[1] << 8) | out[0];
}
