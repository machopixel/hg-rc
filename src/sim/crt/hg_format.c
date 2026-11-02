/* C runtime replacement, formatting: see hg_format.h. */
#include <stdint.h>
#include <string.h>
#include <SDL3/SDL_stdinc.h>
#include "hg_format.h"

/* out of [buf, size): the character at position at, if it fits */
static void hg_put(char *buf, size_t size, size_t at, char c)
{
    if (at + 1 < size)
        buf[at] = c;
}

/* ---- %f: the original's digits ----------------------------------------------------------------------------- */

#define HG_SIG 17       /* the original rounds the exact value to this many significant digits first */
#define HG_LIMBS 130    /* base 1e9 limbs: enough for every double (5^1074 and 2^1024) */

/*
 * The exact decimal digits of |v| (finite) into d, most significant first, without leading zeros ("0" for zero),
 * and the position of the decimal point: v = 0.d1d2d3... * 10^point. Exact: a double is m * 2^e, so the digits
 * are those of m * 2^e (e >= 0) or of m * 5^-e with the point moved -e places (e < 0).
 */
static int hg_exact_digits(double v, char *d, int *point)
{
    uint32_t limb[HG_LIMBS];
    int nl = 1, i, k, nd = 0, shift = 0;
    uint64_t bits, m;
    int e;

    memcpy(&bits, &v, 8);
    e = (int)((bits >> 52) & 0x7ff);
    m = bits & 0xfffffffffffffull;
    if (e == 0)
        e = 1;
    else
        m |= 1ull << 52;
    e -= 1075;
    if (m == 0) {
        d[0] = '0';
        *point = 1;
        return 1;
    }
    limb[0] = (uint32_t)(m % 1000000000u);
    limb[1] = (uint32_t)(m / 1000000000u % 1000000000u);
    limb[2] = (uint32_t)(m / 1000000000u / 1000000000u);
    nl = limb[2] ? 3 : limb[1] ? 2 : 1;
    for (k = 0; k < (e >= 0 ? e : -e); k++) {
        uint32_t carry = 0, f = e >= 0 ? 2u : 5u;

        for (i = 0; i < nl; i++) {
            uint64_t x = (uint64_t)limb[i] * f + carry;

            limb[i] = (uint32_t)(x % 1000000000u);
            carry = (uint32_t)(x / 1000000000u);
        }
        if (carry != 0)
            limb[nl++] = carry;
    }
    if (e < 0)
        shift = -e;
    for (i = nl - 1; i >= 0; i--) {
        char tmp[10];
        int j;

        for (j = 8; j >= 0; j--) {
            tmp[j] = (char)('0' + limb[i] % 10);
            limb[i] /= 10;
        }
        for (j = 0; j < 9; j++)
            if (nd > 0 || tmp[j] != '0' || (i == 0 && j == 8))
                d[nd++] = tmp[j];
    }
    *point = nd - shift;
    return nd;
}

/* The digits rounded at position at (digits after it dropped): half up, or with ties_down an exact half down;
 * returns the new count, point moved on a carry out of the first digit. */
static int hg_round_digits(char *d, int nd, int at, int *point, int ties_down)
{
    int i, up;

    if (at >= nd)
        return nd;
    if (at < 0)
        return 0;
    up = d[at] >= '5';
    if (up && ties_down && d[at] == '5') {
        for (i = at + 1; i < nd && d[i] == '0'; i++)
            ;
        up = i < nd;            /* only above an exact half */
    }
    if (up) {
        for (i = at - 1; i >= 0; i--) {
            if (d[i] != '9') {
                d[i]++;
                break;
            }
            d[i] = '0';
        }
        if (i < 0) {            /* 9...9 rounded up: one digit more in front */
            memmove(d + 1, d, (size_t)at);
            d[0] = '1';
            (*point)++;
            return at + 1;
        }
    }
    return at;
}

/*
 * %f of a finite double as the original prints it (measured against the original's sprintf on this CPU): the exact
 * value rounded to HG_SIG significant digits (an exact half down), that rounded half up to prec places, a negative zero without its sign. Flags: '-' left, '+' / ' ' sign, '0' zeros,
 * '#' a point even for no places.
 */
static int hg_format_f(char *out, size_t cap, double v, int width, int prec, const char *flags)
{
    static char d[1200];
    char body[1400];
    int nd, point, b = 0, i, len, neg = 0, pad;
    char sign = 0;

    if (prec < 0)
        prec = 6;
    if (v < 0) {
        neg = 1;
        v = -v;
    }
    nd = hg_exact_digits(v, d, &point);
    nd = hg_round_digits(d, nd, HG_SIG, &point, 1);     /* (an exact half: down, as the original measured) */
    nd = hg_round_digits(d, nd, point + prec, &point, 0);
    if (nd == 0) {              /* rounded away below the last place: a zero */
        d[0] = '0';
        nd = 1;
        point = 1;
    }
    if (point <= 0) {
        body[b++] = '0';
    } else {
        for (i = 0; i < point && b < 600; i++)
            body[b++] = i < nd ? d[i] : '0';
    }
    if (prec > 0 || strchr(flags, '#') != NULL)
        body[b++] = '.';
    for (i = 0; i < prec && b < (int)sizeof body - 1; i++) {
        int at = point + i;

        body[b++] = at >= 0 && at < nd ? d[at] : '0';
    }
    body[b] = 0;
    if (v == 0.0)
        neg = 0;                /* the original prints a negative zero without its sign */
    if (neg)
        sign = '-';
    else if (strchr(flags, '+') != NULL)
        sign = '+';
    else if (strchr(flags, ' ') != NULL)
        sign = ' ';
    len = b + (sign != 0);
    pad = width > len ? width - len : 0;
    i = 0;
    if (strchr(flags, '-') != NULL) {
        if (sign)
            hg_put(out, cap, (size_t)i++, sign);
        for (b = 0; body[b] != 0; b++)
            hg_put(out, cap, (size_t)i++, body[b]);
        while (pad-- > 0)
            hg_put(out, cap, (size_t)i++, ' ');
    } else if (strchr(flags, '0') != NULL) {
        if (sign)
            hg_put(out, cap, (size_t)i++, sign);
        while (pad-- > 0)
            hg_put(out, cap, (size_t)i++, '0');
        for (b = 0; body[b] != 0; b++)
            hg_put(out, cap, (size_t)i++, body[b]);
    } else {
        while (pad-- > 0)
            hg_put(out, cap, (size_t)i++, ' ');
        if (sign)
            hg_put(out, cap, (size_t)i++, sign);
        for (b = 0; body[b] != 0; b++)
            hg_put(out, cap, (size_t)i++, body[b]);
    }
    if (cap > 0)
        out[(size_t)i < cap ? (size_t)i : cap - 1] = 0;
    return i;
}

int hg_vformat(char *buf, size_t size, const char *fmt, va_list ap)
{
    size_t n = 0;
    const char *p = fmt;

    while (*p != 0) {
        char spec[32], piece[512], fls[12];
        const char *start;
        int nf = 0;
        int space = 0, plus = 0, width = -2, prec = -2, len, k, i, m;
        char conv;

        if (*p != '%') {
            hg_put(buf, size, n++, *p++);
            continue;
        }
        start = p++;
        /* flags */
        k = 0;
        spec[k++] = '%';
        for (;; p++) {
            if (*p == ' ')
                space = 1;
            else if (*p == '+')
                plus = 1, spec[k++] = '+';
            else if (*p == '-' || *p == '#' || *p == '0')
                spec[k++] = *p;
            else
                break;
            if (nf < (int)sizeof fls - 2)
                fls[nf++] = *p;
        }
        /* width and precision ('*' taken from the arguments) */
        if (*p == '*') {
            width = va_arg(ap, int);
            if (width < 0) {    /* a negative width from the arguments: left-aligned */
                spec[k++] = '-';
                fls[nf++] = '-';
                width = -width;
            }
            p++;
        } else if (*p >= '0' && *p <= '9') {
            width = 0;
            while (*p >= '0' && *p <= '9')
                width = width * 10 + (*p++ - '0');
        }
        if (*p == '.') {
            p++;
            prec = 0;
            if (*p == '*') {
                prec = va_arg(ap, int);
                if (prec < 0)   /* a negative precision from the arguments: none */
                    prec = -2;
                p++;
            } else {
                while (*p >= '0' && *p <= '9')
                    prec = prec * 10 + (*p++ - '0');
            }
        }
        if (width != -2)
            k += SDL_snprintf(spec + k, sizeof spec - (size_t)k, "%d", width);
        if (prec != -2)
            k += SDL_snprintf(spec + k, sizeof spec - (size_t)k, ".%d", prec);
        /* size: h, l, ll, I64, L (the game uses h and l) */
        len = 0;
        if (*p == 'h') {
            len = 'h';
            p++;
        } else if (*p == 'l') {
            len = 'l';
            p++;
            if (*p == 'l')
                len = 'L', p++;
        } else if (p[0] == 'I' && p[1] == '6' && p[2] == '4') {
            len = 'L';
            p += 3;
        } else if (*p == 'L') {
            len = 'D';          /* long double: a double here, as MSVC */
            p++;
        }
        fls[nf] = 0;
        conv = *p;
        if (conv == 0) {        /* a cut-off conversion: printed as written */
            while (start < p)
                hg_put(buf, size, n++, *start++);
            break;
        }
        p++;
        if (conv == '%') {
            hg_put(buf, size, n++, '%');
            continue;
        }
        /* the space flag: formatted with '+', then the sign character of a non-negative number becomes a space */
        if (space && !plus && (conv == 'd' || conv == 'i' || conv == 'f' || conv == 'e' || conv == 'E' ||
                               conv == 'g' || conv == 'G')) {
            memmove(spec + 2, spec + 1, (size_t)k - 1);
            spec[1] = '+';
            k++;
        } else {
            space = 0;
        }
        switch (conv) {
        case 'd': case 'i':
            spec[k++] = conv == 'i' ? 'd' : conv;
            spec[k] = 0;
            if (len == 'L') {
                char s2[34];
                memcpy(s2, spec, (size_t)k - 1);
                memcpy(s2 + k - 1, "lld", 4);
                m = SDL_snprintf(piece, sizeof piece, s2, va_arg(ap, long long));
            } else {
                int v = va_arg(ap, int);
                m = SDL_snprintf(piece, sizeof piece, spec, len == 'h' ? (int)(short)v : v);
            }
            break;
        case 'u': case 'x': case 'X': case 'o':
            spec[k++] = conv;
            spec[k] = 0;
            if (len == 'L') {
                char s2[34];
                memcpy(s2, spec, (size_t)k - 1);
                s2[k - 1] = 'l', s2[k] = 'l', s2[k + 1] = conv, s2[k + 2] = 0;
                m = SDL_snprintf(piece, sizeof piece, s2, va_arg(ap, unsigned long long));
            } else {
                unsigned v = va_arg(ap, unsigned);
                m = SDL_snprintf(piece, sizeof piece, spec, len == 'h' ? (unsigned)(unsigned short)v : v);
            }
            break;
        case 'f': case 'e': case 'E': case 'g': case 'G': {
            double v = va_arg(ap, double);

            if (conv == 'f' && v - v == 0.0) {     /* finite %f: the original's digits (hg_format_f) */
                m = hg_format_f(piece, sizeof piece, v, width < 0 ? 0 : width, prec, fls);
                space = 0;
                break;
            }
            spec[k++] = conv;
            spec[k] = 0;
            if (v == 0.0)
                v = 0.0;        /* the original prints a negative zero without its sign */
            m = SDL_snprintf(piece, sizeof piece, spec, v);
            break;
        }
        case 'c':
            spec[k++] = 'c';
            spec[k] = 0;
            m = SDL_snprintf(piece, sizeof piece, spec, va_arg(ap, int));
            break;
        case 's': {
            const char *s = va_arg(ap, const char *);
            spec[k++] = 's';
            spec[k] = 0;
            m = SDL_snprintf(piece, sizeof piece, spec, s != NULL ? s : "(null)");
            if (m >= (int)sizeof piece) {   /* a long text: copied straight, with the same padding rules */
                int l = (int)strlen(s != NULL ? s : "(null)");
                if (prec >= 0 && prec < l)
                    l = prec;
                if (width > l && strchr(spec, '-') == NULL)
                    for (i = 0; i < width - l; i++)
                        hg_put(buf, size, n++, ' ');
                for (i = 0; i < l; i++)
                    hg_put(buf, size, n++, (s != NULL ? s : "(null)")[i]);
                if (width > l && strchr(spec, '-') != NULL)
                    for (i = 0; i < width - l; i++)
                        hg_put(buf, size, n++, ' ');
                continue;
            }
            break;
        }
        case 'p':
            /* MSVC prints a pointer as 8 upper-case hex digits */
            m = SDL_snprintf(piece, sizeof piece, "%08X", (unsigned)(uintptr_t)va_arg(ap, void *));
            break;
        case 'n':
            *va_arg(ap, int *) = (int)n;
            continue;
        default:                /* unknown: printed as written, as MSVC */
            while (start < p)
                hg_put(buf, size, n++, *start++);
            continue;
        }
        if (m < 0)
            m = 0;
        if (m >= (int)sizeof piece)
            m = (int)sizeof piece - 1;
        if (space) {
            for (i = 0; i < m; i++) {
                if (piece[i] == '+') {
                    piece[i] = ' ';
                    break;
                }
                if (piece[i] == '-')
                    break;
            }
        }
        for (i = 0; i < m; i++)
            hg_put(buf, size, n++, piece[i]);
    }
    if (size > 0)
        buf[n < size ? n : size - 1] = 0;
    return (int)n;
}

int hg_vscan(const char *text, const char *fmt, va_list ap)
{
    return SDL_vsscanf(text, fmt, ap);
}
