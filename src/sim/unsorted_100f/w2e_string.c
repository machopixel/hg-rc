#include <string.h>
#include "w2e.h"

// FUNCTION: HGSIM 0x100fc690
char *__cdecl Str_Copy(char *dst, const char *src)
{
    char *p = dst;

    while ((*p++ = *src++) != 0)
        ;
    return dst;
}

// FUNCTION: HGSIM 0x100fc7f0
char *__cdecl Str_CopyN(char *dst, const char *src, uint32_t n)
{
    char *p = dst;

    while (n != 0 && *src != 0) {
        *p++ = *src++;
        n--;
    }
    while (n != 0) {
        *p++ = 0;
        n--;
    }
    return dst;
}

// FUNCTION: HGSIM 0x100fc8f0
uint32_t __cdecl Str_Length(const char *s)
{
    const char *p = s;

    while (*p != 0)
        p++;
    return (uint32_t)(p - s);
}

// FUNCTION: HGSIM 0x100fcab0
void *__cdecl Mem_Fill(void *dst, int c, uint32_t n)
{
    return memset(dst, c, n);   /* C runtime replacement: standard C */
}

// FUNCTION: HGSIM 0x100fcd30
int __cdecl Str_Compare(const char *a, const char *b)
{
    for (;;) {
        uint8_t x = (uint8_t)*a, y = (uint8_t)*b;

        if (x != y)
            return x < y ? -1 : 1;
        if (x == 0)
            return 0;
        a++;
        b++;
    }
}

// FUNCTION: HGSIM 0x100fcdc0
void *__cdecl Mem_Move(void *dst, const void *src, uint32_t n)
{
    return memmove(dst, src, n);    /* C runtime replacement: standard C */
}

// FUNCTION: HGSIM 0x100fe7f0
int __cdecl Str_CompareN(const char *a, const char *b, uint32_t n)
{
    while (n != 0) {
        uint8_t x = (uint8_t)*a, y = (uint8_t)*b;

        if (x != y)
            return x < y ? -1 : 1;
        if (x == 0)
            return 0;
        a++;
        b++;
        n--;
    }
    return 0;
}
