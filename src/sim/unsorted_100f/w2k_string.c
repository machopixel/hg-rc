#include "w2k.h"

// FUNCTION: HGSIM 0x100fd650
char *__cdecl Crt_StrChr(const char *s, int c)
{
    char want = (char)c;

    for (;; s++) {
        if (*s == want)
            return (char *)s;
        if (*s == 0)
            return 0;
    }
}

// FUNCTION: HGSIM 0x100ff160
unsigned __cdecl Crt_StrSpn(const char *s, const char *set)
{
    unsigned char member[256];
    unsigned n = 0;
    int i;

    for (i = 0; i < 256; i++)
        member[i] = 0;
    for (; *set != 0; set++)
        member[(unsigned char)*set] = 1;
    while (*s != 0 && member[(unsigned char)*s]) {
        s++;
        n++;
    }
    return n;
}
