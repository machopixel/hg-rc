#include <string.h>
#include "w6k.h"
#include "../util/access.h"

#define ADDR(p) ((uint32_t)(uintptr_t)(p))
#define MBFLAGS(c) (GBYTES(0x1017eac1)[(c)])   /* flag 4: first byte of a two-byte character */

// FUNCTION: HGSIM 0x101078d0
int32_t __cdecl Sim_101078d0(const char *a, const char *b, uint32_t n)
{
    int r;

    if (n == 0)
        return 0;
    r = W6K_FN_1010afa0(G32(0x1017ebc8), 1, a, n, b, n, G32(0x1017ebc4));
    if (r == 0)
        return 0x7fffffff;
    return r - 2;
}

// FUNCTION: HGSIM 0x1010b4b0
int32_t __cdecl Sim_1010b4b0(const char *name, uint32_t len)
{
    uint32_t p = G32(0x1017e8b0);
    uint32_t e;
    uint8_t c;

    for (e = U32(p, 0); e != 0; e = U32(p, 0)) {
        if (Sim_101078d0(name, (const char *)(uintptr_t)e, len) == 0) {
            c = U8(U32(p, 0), len);
            if (c == '=' || c == 0)
                return (int32_t)(p - G32(0x1017e8b0)) >> 2;
        }
        p += 4;
    }
    return -((int32_t)(p - G32(0x1017e8b0)) >> 2);
}

// FUNCTION: HGSIM 0x1010c9f0
char *__cdecl Sim_1010c9f0(const char *s)
{
    char *d;

    if (s == 0)
        return 0;
    d = W6K_FN_100fef30((uint32_t)strlen(s) + 1);
    if (d == 0)
        return 0;
    memcpy(d, s, strlen(s) + 1);
    return d;
}

// FUNCTION: HGSIM 0x1010d240
int32_t __cdecl Sim_1010d240(const uint8_t *a, const uint8_t *b, uint32_t n)
{
    if (n == 0)
        return 0;
    if (G32(0x1017e950) == 0) {
        uint8_t ca, cb;

        do {
            ca = *a;
            cb = *b;
            if (ca == 0 || cb == 0)
                break;
            a++;
            b++;
            if (ca >= 'A' && ca <= 'Z')
                ca += 0x20;
            if (cb >= 'A' && cb <= 'Z')
                cb += 0x20;
            if (ca != cb)
                return ca < cb ? -1 : 1;
        } while (--n != 0);
        if (ca == cb)
            return 0;
        return ca < cb ? -1 : 1;
    } else {
        /* the original loads only the low byte of each character into the registers that hold the converted
         * values, so the upper 24 bits of the previous results go along to 0x100ff280 and into the compare */
        uint32_t ca = 0, cb = 0;

        do {
            ca = (ca & 0xffffff00u) | *a;
            cb = (cb & 0xffffff00u) | *b;
            if (ca == 0 || cb == 0)
                break;
            a++;
            b++;
            cb = W6K_FN_100ff280(cb);
            ca = W6K_FN_100ff280(ca);
            if (ca != cb)
                return ca < cb ? -1 : 1;
        } while (--n != 0);
        if (ca == cb)
            return 0;
        return ca < cb ? -1 : 1;
    }
}

// FUNCTION: HGSIM 0x1010d340
char *__cdecl Crt_StrNSet(char *s, uint32_t c, uint32_t n)
{
    uint32_t k = 0;

    while (k < n && s[k] != 0)
        k++;
    memset(s, (int)(uint8_t)c, k);
    return s;
}

// FUNCTION: HGSIM 0x10107b20
void __cdecl Sim_10107b20(const uint8_t *cmd, uint32_t *argv, uint8_t *args, int32_t *nargs, int32_t *nchars)
{
    const uint8_t *p = cmd;
    uint32_t c;
    uint32_t inquote;
    uint32_t copy;
    uint32_t nbs;

    *nchars = 0;
    *nargs = 1;
    if (argv != 0)
        *argv++ = ADDR(args);
    /* the program name: up to the closing quote, or up to a blank */
    if (*p == '"') {
        p++;
        c = *p;
        while (c != '"' && c != 0) {
            if ((MBFLAGS(c) & 4) != 0) {
                ++*nchars;
                /* without a buffer the first byte is not stepped over (as in the original) */
                if (args != 0) {
                    *args++ = *p;
                    p++;
                }
            }
            ++*nchars;
            if (args != 0)
                *args++ = *p;
            p++;
            c = *p;
        }
        ++*nchars;
        if (args != 0)
            *args++ = 0;
        if (*p == '"')
            p++;
    } else {
        do {
            ++*nchars;
            if (args != 0)
                *args++ = *p;
            c = *p++;
            if ((MBFLAGS(c) & 4) != 0) {
                ++*nchars;
                if (args != 0)
                    *args++ = *p;
                p++;
            }
        } while (c != ' ' && c != 0 && c != '\t');
        if (c == 0)
            p--;
        else if (args != 0)
            args[-1] = 0;
    }
    /* the arguments */
    inquote = 0;
    for (;;) {
        if (*p == 0)
            break;
        while (*p == ' ' || *p == '\t')
            p++;
        if (*p == 0)
            break;
        if (argv != 0)
            *argv++ = ADDR(args);
        ++*nargs;
        for (;;) {
            copy = 1;
            nbs = 0;
            while (*p == '\\') {
                p++;
                nbs++;
            }
            if (*p == '"') {
                if ((nbs & 1) == 0) {
                    if (inquote != 0 && p[1] == '"')
                        p++;
                    else
                        copy = 0;
                    inquote = inquote == 0;
                }
                nbs >>= 1;
            }
            while (nbs != 0) {
                if (args != 0)
                    *args++ = '\\';
                ++*nchars;
                nbs--;
            }
            c = *p;
            if (c == 0 || (inquote == 0 && (c == ' ' || c == '\t')))
                break;
            if (copy != 0) {
                if (args != 0) {
                    if ((MBFLAGS(c) & 4) != 0) {
                        *args++ = (uint8_t)c;
                        p++;
                        ++*nchars;
                    }
                    *args++ = *p;
                    ++*nchars;
                } else {
                    if ((MBFLAGS(c) & 4) != 0) {
                        p++;
                        ++*nchars;
                    }
                    ++*nchars;
                }
            }
            p++;
        }
        if (args != 0)
            *args++ = 0;
        ++*nchars;
    }
    if (argv != 0)
        *argv = 0;
    ++*nargs;
}

// FUNCTION: HGSIM 0x10108e60
void __cdecl Sim_10108e60(char *buf, int32_t ndig, uint8_t *st)
{
    const char *m = (const char *)(uintptr_t)U32(st, 0xc);
    char *p = buf + 1;
    int32_t left = ndig;

    buf[0] = '0';
    for (; left > 0; left--) {
        if (*m != 0)
            *p++ = *m++;
        else
            *p++ = '0';
    }
    *p = 0;
    if (left >= 0 && (signed char)*m >= '5') {
        p--;
        while (*p == '9') {
            *p = '0';
            p--;
        }
        ++*p;
    }
    if (buf[0] == '1')
        S32(st, 4)++;
    else
        memmove(buf, buf + 1, strlen(buf + 1) + 1);
}
