#include "w2e.h"
#include "../unsorted_100f/w2e.h"
#include "../util/access.h"

// FUNCTION: HGSIM 0x100b6b87
int __cdecl Str_CompareIndirect(const char *const *a, const char *const *b)
{
    return Str_Compare(*a, *b);
}

// FUNCTION: HGSIM 0x100b971d
int __cdecl Sim_100b971d(const char *a, const char *b)
{
    return Str_CompareN(a + 0x18, b, 4);
}

// FUNCTION: HGSIM 0x100b9db0
int __cdecl Sim_100b9db0(const char *a, const char *b)
{
    return Str_CompareN(a + 0x18, b + 0x18, 4);
}

// FUNCTION: HGSIM 0x100bcb52
const char *__cdecl Sim_100bcb52(const char *key, const void *list)
{
    int i;

    for (i = 0; i < S32(list, 4); i++) {
        const char *s = (const char *)BYTES(U32(U32(list, 8), (uint32_t)i * 4));

        if (Str_CompareN(key, s, 4) == 0)
            return s;
    }
    return 0;
}

// FUNCTION: HGSIM 0x100bcba7
int __cdecl Sim_100bcba7(const char *a, const char *b)
{
    return Str_CompareN(a, b, 8);
}

// FUNCTION: HGSIM 0x100bccff
void __cdecl Path_ReplaceExtension(char *path, const char *ext)
{
    int len = (int)Str_Length(path);
    char *p = path + len;

    while (len != 0) {
        if (*p == '.') {
            Str_Copy(p + 1, ext);
            return;
        }
        p--;
        len--;
    }
}
