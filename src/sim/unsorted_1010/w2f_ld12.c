#include "w2f.h"
#include "../util/access.h"

// FUNCTION: HGSIM 0x1010b5c0
int __cdecl Sim_1010b5c0(uint32_t a, uint32_t b, uint32_t *out)
{
    uint32_t sum = b + a;
    int carry = sum < a || sum < b;

    *out = sum;
    return carry;
}

// FUNCTION: HGSIM 0x1010b5f0
int __cdecl Sim_1010b5f0(uint32_t *a, const uint32_t *b)
{
    if (Sim_1010b5c0(a[0], b[0], &a[0]) != 0) {
        if (Sim_1010b5c0(a[1], 1, &a[1]) != 0)
            a[2]++;
    }
    if (Sim_1010b5c0(a[1], b[1], &a[1]) != 0)
        a[2]++;
    return Sim_1010b5c0(a[2], b[2], &a[2]);
}

// FUNCTION: HGSIM 0x1010b660
void __cdecl Sim_1010b660(uint32_t *p)
{
    uint32_t w0 = p[0];
    uint32_t w1 = p[1];
    uint32_t w2 = p[2];

    p[0] = w0 + w0;
    p[1] = (w1 + w1) | (w0 >> 31);
    p[2] = (w2 << 1) | (w1 >> 31);
}

// FUNCTION: HGSIM 0x1010b690
void __cdecl Sim_1010b690(uint32_t *p)
{
    uint32_t w0 = p[0];
    uint32_t w1 = p[1];
    uint32_t w2 = p[2];

    p[1] = (w1 >> 1) | (w2 << 31);
    p[2] = w2 >> 1;
    p[0] = (w0 >> 1) | (w1 << 31);
}

// FUNCTION: HGSIM 0x1010b6c0
void __cdecl Sim_1010b6c0(const char *digits, uint32_t count, uint32_t *out)
{
    uint32_t exp = 0x404e;
    uint32_t t[3];

    out[0] = 0;
    out[1] = 0;
    out[2] = 0;
    while (count != 0) {
        t[0] = out[0];
        t[1] = out[1];
        t[2] = out[2];
        Sim_1010b660(out);
        Sim_1010b660(out);
        Sim_1010b5f0(out, t);
        Sim_1010b660(out);
        t[0] = (uint32_t)(int32_t)*digits;
        t[1] = 0;
        t[2] = 0;
        Sim_1010b5f0(out, t);
        digits++;
        count--;
    }
    while (out[2] == 0) {
        uint32_t w1 = out[1];
        uint32_t w0 = out[0];

        exp += 0xfff0;
        out[2] = w1 >> 16;
        out[1] = (w0 >> 16) | (w1 << 16);
        out[0] = w0 << 16;
    }
    while ((out[2] & 0x8000) == 0) {
        Sim_1010b660(out);
        exp += 0xffff;
    }
    U16(out, 0xa) = (uint16_t)exp;
}

// FUNCTION: HGSIM 0x1010ca40
void __cdecl Sim_1010ca40(void *a, const void *b)
{
    uint32_t wa = U16(a, 0xa);
    uint32_t wb = U16(b, 0xa);
    uint32_t ea = wa & 0x7fff;
    uint32_t eb = wb & 0x7fff;
    uint32_t sign = (wa ^ wb) & 0x8000;
    uint32_t exp = ea + eb;
    uint32_t sticky = 0;
    uint32_t prod[3] = {0, 0, 0};
    uint32_t pp;
    int i;
    int cnt;

    if (ea >= 0x7fff || eb >= 0x7fff || (uint16_t)exp > 0xbffd) {
        U32(a, 0) = 0;
        U32(a, 4) = 0;
        U32(a, 8) = (sign != 0 ? 0x80000000u : 0) + 0x7fff8000u;
        return;
    }
    if ((uint16_t)exp <= 0x3fbf) {
        U32(a, 8) = 0;
        U32(a, 4) = 0;
        U32(a, 0) = 0;
        return;
    }
    if (ea == 0) {
        exp++;
        if ((U32(a, 8) & 0x7fffffff) == 0 && U32(a, 4) == 0 && U32(a, 0) == 0) {
            U16(a, 0xa) = 0;
            return;
        }
    }
    if (eb == 0) {
        exp++;
        if ((U32(b, 8) & 0x7fffffff) == 0 && U32(b, 4) == 0 && U32(b, 0) == 0) {
            U32(a, 8) = 0;
            U32(a, 4) = 0;
            U32(a, 0) = 0;
            return;
        }
    }
    pp = 4;
    for (i = 0, cnt = 5; cnt > 0; i++, cnt--) {
        uint32_t pa = 2 * (uint32_t)i;
        uint32_t pb = 8;
        int j;

        for (j = cnt; j > 0; j--) {
            uint32_t p = (uint32_t)U16(b, pb) * (uint32_t)U16(a, pa);

            if (Sim_1010b5c0(U32(prod, pp - 4), p, (uint32_t *)((uint8_t *)prod + (pp - 4))) != 0)
                U16(prod, pp) = (uint16_t)(U16(prod, pp) + 1);
            pa += 2;
            pb -= 2;
        }
        pp += 2;
    }
    exp += 0xc002;
    while ((int16_t)exp > 0 && (U32(prod, 8) & 0x80000000u) == 0) {
        Sim_1010b660(prod);
        exp += 0xffff;
    }
    if ((int16_t)exp <= 0) {
        exp += 0xffff;
        if ((int16_t)exp < 0) {
            int32_t shift = -(int16_t)exp;

            exp += (uint32_t)shift;
            do {
                if ((U8(prod, 0) & 1) != 0)
                    sticky++;
                Sim_1010b690(prod);
                shift--;
            } while (shift != 0);
        }
        if (sticky != 0)
            U8(prod, 0) |= 1;
    }
    if (U16(prod, 0) > 0x8000 || (U32(prod, 0) & 0x1ffff) == 0x18000) {
        if (U32(prod, 2) != 0xffffffffu) {
            U32(prod, 2) = U32(prod, 2) + 1;
        } else {
            U32(prod, 2) = 0;
            if (U32(prod, 6) != 0xffffffffu) {
                U32(prod, 6) = U32(prod, 6) + 1;
            } else {
                U32(prod, 6) = 0;
                if (U16(prod, 10) != 0xffff) {
                    U16(prod, 10) = (uint16_t)(U16(prod, 10) + 1);
                } else {
                    U16(prod, 10) = 0x8000;
                    exp++;
                }
            }
        }
    }
    if ((uint16_t)exp >= 0x7fff) {
        U32(a, 4) = 0;
        U32(a, 0) = 0;
        U32(a, 8) = (sign != 0 ? 0x80000000u : 0) + 0x7fff8000u;
        return;
    }
    U16(a, 0) = U16(prod, 2);
    U32(a, 2) = U32(prod, 4);
    U32(a, 6) = U32(prod, 8);
    U16(a, 0xa) = (uint16_t)(exp | sign);
}

// FUNCTION: HGSIM 0x1010cd00
void __cdecl Sim_1010cd00(void *a, int n, int flag)
{
    uint32_t tab = 0x1015c1d0 - 0x60;
    int32_t m = n;

    if (m == 0)
        return;
    if (m < 0) {
        tab = 0x1015c330 - 0x60;
        m = (int32_t)(0u - (uint32_t)m);
    }
    if (flag == 0)
        U16(a, 0) = 0;
    while (m != 0) {
        uint32_t idx = (uint32_t)m & 7;
        uint32_t entry;
        uint32_t copy[3];

        tab += 0x54;
        m >>= 3;
        if (idx == 0)
            continue;
        entry = tab + idx * 12;
        if (U16(GBYTES(entry), 0) >= 0x8000) {
            copy[0] = U32(GBYTES(entry), 0);
            copy[1] = U32(GBYTES(entry), 4);
            copy[2] = U32(GBYTES(entry), 8);
            U32(copy, 2) = U32(copy, 2) - 1;
            Sim_1010ca40(a, copy);
        } else {
            Sim_1010ca40(a, GBYTES(entry));
        }
    }
}
