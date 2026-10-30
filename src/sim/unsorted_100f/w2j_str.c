#include "w2j.h"

// FUNCTION: HGSIM 0x100fc6a0
char *__cdecl Str_Cat(char *dst, const char *src)
{
    char *end = dst;

    while (*end != '\0')
        end++;
    while ((*end++ = *src++) != '\0')
        ;
    return dst;
}

// FUNCTION: HGSIM 0x100fd870
char *__cdecl Str_NCat(char *dst, const char *src, unsigned count)
{
    char *end = dst;

    if (count == 0)
        return dst;
    while (*end != '\0')
        end++;
    while (count != 0) {
        char c = *src++;

        *end++ = c;
        if (c == '\0')
            return dst;
        count--;
    }
    *end = '\0';
    return dst;
}

// FUNCTION: HGSIM 0x100ff1a0
unsigned __cdecl Str_CSpan(const char *s, const char *set)
{
    unsigned char member[256];
    unsigned count = 0;
    int i;

    for (i = 0; i < 256; i++)
        member[i] = 0;
    for (; *set != '\0'; set++)
        member[(unsigned char)*set] = 1;
    while (*s != '\0' && !member[(unsigned char)*s]) {
        s++;
        count++;
    }
    return count;
}

// FUNCTION: HGSIM 0x100ffdb0
char *__cdecl Str_FindLast(const char *s, int c)
{
    const char *end = s;
    char want = (char)c;

    while (*end != '\0')
        end++;
    for (;; end--) {
        if (*end == want)
            return (char *)end;
        if (end == s)
            return 0;
    }
}
