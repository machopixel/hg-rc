/* Minimal runtime for the flat test binaries (no C library there). */
#include <stddef.h>

unsigned char *g_simBase;

void *memset(void *d, int c, size_t n)
{
    unsigned char *p = d;
    while (n--)
        *p++ = (unsigned char)c;
    return d;
}

void *memcpy(void *d, const void *s, size_t n)
{
    unsigned char *p = d;
    const unsigned char *q = s;
    while (n--)
        *p++ = *q++;
    return d;
}

void *memmove(void *d, const void *s, size_t n)
{
    unsigned char *p = d;
    const unsigned char *q = s;
    if (p < q) {
        while (n--)
            *p++ = *q++;
    } else {
        p += n;
        q += n;
        while (n--)
            *--p = *--q;
    }
    return d;
}

size_t strlen(const char *s)
{
    size_t n = 0;
    while (s[n])
        n++;
    return n;
}

int memcmp(const void *a, const void *b, size_t n)
{
    const unsigned char *p = a, *q = b;
    for (; n; n--, p++, q++)
        if (*p != *q)
            return *p - *q;
    return 0;
}

int strcmp(const char *a, const char *b)
{
    while (*a && *a == *b)
        a++, b++;
    return (unsigned char)*a - (unsigned char)*b;
}

/* Stand-ins for the platform math library that the interpreted functions call (see DECISIONS.md): the x87
 * result rounded to double, as a 64-bit libm returns it. */
static double rt_double(long double v)
{
    volatile double d = (double)v;
    return d;
}

static long double rt_patan(long double y, long double x)
{
    long double r;
    __asm__("fpatan" : "=t"(r) : "0"(x), "u"(y) : "st(1)");
    return r;
}

static long double rt_sqrt(long double x)
{
    long double r;
    __asm__("fsqrt" : "=t"(r) : "0"(x));
    return r;
}

double sin(double x)
{
    long double r;
    __asm__("fsin" : "=t"(r) : "0"((long double)x));
    return rt_double(r);
}

double cos(double x)
{
    long double r;
    __asm__("fcos" : "=t"(r) : "0"((long double)x));
    return rt_double(r);
}

double atan2(double y, double x)
{
    return rt_double(rt_patan(y, x));
}

double asin(double x)
{
    return rt_double(rt_patan(x, rt_sqrt((1.0L - x) * (1.0L + x))));
}

double acos(double x)
{
    return rt_double(rt_patan(rt_sqrt((1.0L - x) * (1.0L + x)), x));
}

int _stricmp(const char *a, const char *b)
{
    for (;; a++, b++) {
        int x = (unsigned char)*a, y = (unsigned char)*b;
        if (x >= 'A' && x <= 'Z') x += 32;
        if (y >= 'A' && y <= 'Z') y += 32;
        if (x != y || x == 0)
            return x - y;
    }
}
