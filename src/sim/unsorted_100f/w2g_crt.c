#include "w2g.h"
#include "../simmem.h"
#include <string.h>

// FUNCTION: HGSIM 0x100fd100
int __cdecl Math_AbsInt_100fd100(int value)
{
    return value < 0 ? (int)(0u - (unsigned)value) : value;
}

#define IN_MAP(map, c) (((map)[(unsigned char)(c) >> 3] & (1u << ((unsigned char)(c) & 7))) != 0)

// FUNCTION: HGSIM 0x100fd560
char *__cdecl Str_NextToken_100fd560(char *str, const char *delim)
{
    uint8_t map[32];
    char *s;
    char *start;

    memset(map, 0, sizeof map);
    do {
        map[(unsigned char)*delim >> 3] |= (uint8_t)(1u << ((unsigned char)*delim & 7));
    } while (*delim++ != 0);
    s = str != 0 ? str : (char *)(uintptr_t)*SIM_PTR(uint32_t, 0x1017E884);
    while (*s != 0 && IN_MAP(map, *s))
        s++;
    start = s;
    if (*s != 0) {
        for (;;) {
            if (IN_MAP(map, *s)) {
                *s++ = 0;
                break;
            }
            s++;
            if (*s == 0)
                break;
        }
    }
    *SIM_PTR(uint32_t, 0x1017E884) = (uint32_t)(uintptr_t)s;
    return start != s ? start : 0;
}
