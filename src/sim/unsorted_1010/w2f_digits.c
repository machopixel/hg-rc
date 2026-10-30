#include "w2f.h"
#include "../util/access.h"

/* Copies n bytes of the DLL's read-only text constants at addr. */
static void CopyText(uint8_t *dst, uint32_t addr, int n)
{
    int i;

    for (i = 0; i < n; i++)
        dst[i] = U8(GBYTES(addr), i);
}

// FUNCTION: HGSIM 0x1010bfa0
int __cdecl Sim_1010bfa0(uint32_t lo, uint32_t hi, uint32_t expw, int ndigits, uint32_t flag, char *out)
{
    /* 0.1 as an ld12: ten bytes 0xcc and the exponent word 0x3ffb */
    uint32_t tenth[3] = {0xccccccccu, 0xccccccccu, 0x3ffbccccu};
    uint32_t s[3];
    uint32_t t[3];
    uint8_t *o = (uint8_t *)out;
    uint32_t e = expw & 0x7fff;
    int sign = (expw & 0x8000) != 0;
    uint32_t a;
    int32_t dec;
    int count;
    int ebp;
    int32_t ex;
    uint8_t *start;
    uint8_t *w;
    uint8_t *p;
    int8_t last;

    o[2] = sign ? '-' : ' ';
    if (e == 0 && hi == 0 && lo == 0) {
        U16(o, 0) = 0;
        goto zero;
    }
    if (e == 0x7fff) {
        U16(o, 0) = 1;
        if (!(hi == 0x80000000u && lo == 0) && (hi & 0x40000000) == 0) {
            CopyText(o + 4, 0x1012c4b0, 7);
            o[3] = 6;
            return 0;
        }
        if (sign && hi == 0xc0000000u && lo == 0) {
            CopyText(o + 4, 0x1012c4a8, 6);
            o[3] = 5;
            return 0;
        }
        if (hi == 0x80000000u && lo == 0) {
            CopyText(o + 4, 0x1012c4a0, 6);
            o[3] = 5;
            return 0;
        }
        CopyText(o + 4, 0x1012c498, 7);
        o[3] = 6;
        return 0;
    }
    a = (e >> 8) + 2 * (hi >> 24);
    dec = (int32_t)(77 * a + 19728 * e - 0x134312f4u) >> 16;
    U16(s, 10) = (uint16_t)e;
    U32(s, 6) = hi;
    U32(s, 2) = lo;
    U16(s, 0) = 0;
    Sim_1010cd00(s, -(int16_t)dec, 1);
    if (U16(s, 10) >= 0x3fff) {
        dec++;
        Sim_1010ca40(s, tenth);
    }
    U16(o, 0) = (uint16_t)dec;
    if (flag & 1) {
        ebp = ndigits + (int16_t)dec;
        if (ebp <= 0) {
            U16(o, 0) = 0;
            goto zero;
        }
    } else {
        ebp = ndigits;
    }
    if (ebp > 0x15)
        ebp = 0x15;
    ex = (int32_t)U16(s, 10) - 0x3ffe;
    U16(s, 10) = 0;
    for (count = 0; count < 8; count++)
        Sim_1010b660(s);
    if (ex < 0) {
        uint32_t c = (uint32_t)(-ex) & 0xff;

        while (c != 0) {
            Sim_1010b690(s);
            c--;
        }
    }
    start = o + 4;
    w = start;
    for (count = ebp + 1; count > 0; count--) {
        t[0] = s[0];
        t[1] = s[1];
        t[2] = s[2];
        Sim_1010b660(s);
        Sim_1010b660(s);
        Sim_1010b5f0(s, t);
        Sim_1010b660(s);
        *w++ = (uint8_t)(U8(s, 11) + '0');
        U8(s, 11) = 0;
    }
    last = (int8_t)w[-1];
    p = w - 2;
    if (last >= '5') {
        int done = 0;

        if (p >= start) {
            while (*p == '9') {
                *p = '0';
                p--;
                if (p < start)
                    break;
            }
            if (p >= start) {
                (*p)++;
                done = 1;
            }
        }
        if (!done) {
            p++;
            U16(o, 0) = (uint16_t)(U16(o, 0) + 1);
            (*p)++;
        }
    } else {
        while (p >= start && *p == '0')
            p--;
        if (p < start) {
            *start = '0';
            U16(o, 0) = 0;
            o[2] = ' ';
            o[3] = 1;
            o[5] = 0;
            return 1;
        }
    }
    {
        uint8_t n = (uint8_t)((uint8_t)(uintptr_t)p - (uint8_t)(uintptr_t)o - 3);

        o[3] = n;
        o[4 + (int8_t)n] = 0;
    }
    return 1;

zero:
    o[2] = ' ';
    o[3] = 1;
    o[4] = '0';
    o[5] = 0;
    return 1;
}

// FUNCTION: HGSIM 0x10108f00
void *__cdecl Sim_10108f00(uint32_t lo, uint32_t hi)
{
    uint32_t in[2];
    uint32_t ld[3];
    uint8_t *buf = SIM_PTR(uint8_t, 0x1017ebf0);
    int ret;

    in[0] = lo;
    in[1] = hi;
    ld[2] = 0;
    Sim_10108f80(ld, in);
    ret = Sim_1010bfa0(ld[0], ld[1], ld[2] & 0xffff, 0x11, 0, (char *)buf);
    G32(0x1017ec18) = (uint32_t)ret;
    G32(0x1017ec10) = (uint32_t)(int32_t)(int8_t)U8(buf, 2);
    G32(0x1017ec14) = (uint32_t)(int32_t)(int16_t)U16(buf, 0);
    G32(0x1017ec1c) = (uint32_t)(uintptr_t)(buf + 4);
    return GBYTES(0x1017ec10);
}
