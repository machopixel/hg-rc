#include "w2f.h"
#include "../util/access.h"

// FUNCTION: HGSIM 0x10108840
uint32_t __cdecl Sim_10108840(uint32_t flags)
{
    uint32_t r = 0;

    if (flags & 1)
        r = 0x10;
    if (flags & 4)
        r |= 8;
    if (flags & 8)
        r |= 4;
    if (flags & 0x10)
        r |= 2;
    if (flags & 0x20)
        r |= 1;
    if (flags & 2)
        r |= 0x80000;
    return r;
}

// FUNCTION: HGSIM 0x10108880
int __cdecl Sim_10108880(const uint32_t *bits, int n)
{
    int q = n / 32;
    int rem = n % 32;
    uint32_t mask = ~(0xFFFFFFFFu << ((31 - rem) & 31));

    if (bits[q] & mask)
        return 0;
    for (q++; q < 3; q++) {
        if (bits[q] != 0)
            return 0;
    }
    return 1;
}

// FUNCTION: HGSIM 0x101088f0
int __cdecl Sim_101088f0(uint32_t *bits, int n)
{
    int q = n / 32;
    int rem = n % 32;
    int carry = Sim_1010b5c0(bits[q], 1u << ((31 - rem) & 31), &bits[q]);

    for (q--; q >= 0 && carry != 0; q--)
        carry = Sim_1010b5c0(bits[q], 1, &bits[q]);
    return carry;
}

// FUNCTION: HGSIM 0x10108960
int __cdecl Sim_10108960(uint32_t *bits, int n)
{
    int q = n / 32;
    int rem = n % 32;
    unsigned sh = (31 - rem) & 31;
    int result = 0;
    int i;

    if (bits[q] & (1u << sh)) {
        if (Sim_10108880(bits, n + 1) == 0)
            result = Sim_101088f0(bits, n - 1);
    }
    bits[q] &= 0xFFFFFFFFu << sh;
    for (i = q + 1; i < 3; i++)
        bits[i] = 0;
    return result;
}

// FUNCTION: HGSIM 0x10108a00
void __cdecl Sim_10108a00(uint32_t *dst, const uint32_t *src)
{
    int i;

    for (i = 0; i < 3; i++)
        dst[i] = src[i];
}

// FUNCTION: HGSIM 0x10108a20
void __cdecl Sim_10108a20(uint32_t *p)
{
    p[0] = 0;
    p[1] = 0;
    p[2] = 0;
}

// FUNCTION: HGSIM 0x10108a30
int __cdecl Sim_10108a30(const uint32_t *p)
{
    int i;

    for (i = 0; i < 3; i++) {
        if (p[i] != 0)
            return 0;
    }
    return 1;
}

// FUNCTION: HGSIM 0x10108a50
void __cdecl Sim_10108a50(uint32_t *p, int n)
{
    int q = n / 32;
    int r = n % 32;
    uint32_t lowmask = ~(0xFFFFFFFFu << (r & 31));
    unsigned back = (32 - r) & 31;
    uint32_t carry = 0;
    int i;
    int j;

    for (i = 0; i < 3; i++) {
        uint32_t w = p[i];

        p[i] = (w >> (r & 31)) | carry;
        carry = (w & lowmask) << back;
    }
    for (j = 2; j >= 0; j--) {
        if (j >= q)
            p[j] = p[j - q];
        else
            p[j] = 0;
    }
}

// FUNCTION: HGSIM 0x10108b10
int __cdecl Sim_10108b10(const uint16_t *src, uint32_t *dst, const int32_t *fmt)
{
    uint32_t a[3];
    uint32_t b[3];
    uint32_t w = U16(src, 0xa);
    uint32_t sign = w & 0x8000;
    int32_t exp = (int32_t)(w & 0x7fff) - 0x3fff;
    uint32_t bits;
    int ret;

    a[0] = U32(src, 6);
    a[1] = U32(src, 2);
    a[2] = (uint32_t)U16(src, 0) << 16;
    if (exp == -0x3fff) {
        bits = 0;
        if (Sim_10108a30(a) == 0) {
            Sim_10108a20(a);
            ret = 2;
        } else {
            ret = 0;
        }
    } else {
        Sim_10108a00(b, a);
        if (Sim_10108960(a, fmt[2]) != 0)
            exp++;
        if (exp < fmt[1] - fmt[2]) {
            Sim_10108a20(a);
            bits = 0;
            ret = 2;
        } else if (exp <= fmt[1]) {
            Sim_10108a00(a, b);
            Sim_10108a50(a, fmt[1] - exp);
            Sim_10108960(a, fmt[2]);
            Sim_10108a50(a, fmt[3] + 1);
            bits = 0;
            ret = 2;
        } else if (exp >= fmt[0]) {
            Sim_10108a20(a);
            a[0] = 0x80000000u;
            Sim_10108a50(a, fmt[3]);
            bits = (uint32_t)(fmt[5] + fmt[0]);
            ret = 1;
        } else {
            bits = (uint32_t)(fmt[5] + exp);
            a[0] &= 0x7fffffffu;
            Sim_10108a50(a, fmt[3]);
            ret = 0;
        }
    }
    bits <<= ((31 - fmt[3]) & 31);
    bits |= sign != 0 ? 0x80000000u : 0;
    bits |= a[0];
    if (fmt[4] == 0x40) {
        dst[1] = bits;
        dst[0] = a[1];
    } else if (fmt[4] == 0x20) {
        dst[0] = bits;
    }
    return ret;
}

// FUNCTION: HGSIM 0x10108ce0
int __cdecl Sim_10108ce0(const uint16_t *src, uint32_t *dst)
{
    return Sim_10108b10(src, dst, SIM_PTR(int32_t, 0x1015bfe8));
}

// FUNCTION: HGSIM 0x10108d00
int __cdecl Sim_10108d00(const uint16_t *src, uint32_t *dst)
{
    return Sim_10108b10(src, dst, SIM_PTR(int32_t, 0x1015c000));
}

// FUNCTION: HGSIM 0x10108f80
void __cdecl Sim_10108f80(uint32_t *out, const uint32_t *in)
{
    uint32_t hiw = U16(in, 6);
    uint32_t mhi = in[1] & 0xfffff;
    uint32_t mlo = in[0];
    uint32_t e = (hiw >> 4) & 0x7ff;
    uint32_t sign = hiw & 0x8000;
    uint32_t top = 0x80000000u;
    uint32_t se;
    uint32_t hi;
    uint32_t lo;

    if (e == 0) {
        top = 0;
        if (mhi == 0 && mlo == 0) {
            out[1] = 0;
            out[0] = 0;
            U16(out, 8) = 0;
            return;
        }
        se = e + 0x3c01;
    } else if (e == 0x7ff) {
        se = 0x7fff;
    } else {
        se = e + 0x3c00;
    }
    hi = (mlo >> 21) | (mhi << 11) | top;
    lo = mlo << 11;
    out[1] = hi;
    out[0] = lo;
    while ((hi & 0x80000000u) == 0) {
        uint32_t l = lo;

        hi = (l >> 31) | (hi + hi);
        lo = l + l;
        se += 0xffff;
        out[1] = hi;
        out[0] = lo;
    }
    se |= sign;
    U16(out, 8) = (uint16_t)se;
}
