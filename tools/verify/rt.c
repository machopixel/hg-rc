/* Minimal runtime for the flat test binaries (no C library there). */
#include <stddef.h>

unsigned char *g_simBase;
/* 1.2's network receive guard and 10-player tables (the game's own definitions are in
   src/sim/unsorted_1004/w34_netover.c): here so every checker build of a network function links them, whatever its
   source list (closure() then never pulls w34_netover.c in) */
unsigned int g_w34NetOver;
unsigned int g_net10Heard[10], g_net10Db20[10], g_net10Left[10];
int g_net10SnStamp[10];
unsigned char g_net10Mark[10], g_net10Ready[10], g_net10Row[10][0x16], g_net10DiSent[10][0xd1];
unsigned char *g_net10DiBuf;

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

/* The C library heap for crt/hg_heap.c (C runtime replacement): a bump allocator in the image, the same addresses in
   every run (the checker puts the image back between the original's and the C's run). free keeps the space. */
static unsigned char rt_arena[0x40000] __attribute__((aligned(16)));
static size_t rt_top;

void *malloc(size_t n)
{
    void *p;

    n = (n + 15) & ~(size_t)15;
    if (n > sizeof rt_arena - rt_top)
        return 0;
    p = rt_arena + rt_top;
    rt_top += n;
    return p;
}

void *calloc(size_t c, size_t n)
{
    unsigned char *p = malloc(c * n);

    if (p)
        memset(p, 0, c * n);
    return p;
}

void *realloc(void *old, size_t n)
{
    unsigned char *p = malloc(n);

    if (p && old)
        memmove(p, old, n);     /* may read past the old block: still inside the arena */
    return p;
}

void free(void *p)
{
    (void)p;
}
