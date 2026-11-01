#include "w7r.h"
#include "w6h.h"
#include "../util/access.h"
#include "w6i.h"
#include "w7q.h"
#include <string.h>

#define W7R_CTYPE() G32(0x10159450)

// FUNCTION: HGSIM 0x100ff280
int __cdecl Crt_ToLower(int c)
{
    uint8_t in[4];
    uint8_t out[4];
    int n;
    int got;

    if (G32(0x1017e950) == 0) {
        if (c >= 0x41 && c <= 0x5a)
            return c + 0x20;
        return c;
    }
    if (c < 0x100) {
        int upper;

        if (GS32(0x1015965c) > 1)
            upper = Crt_IsCType(c, 1);
        else
            upper = U8(W7R_CTYPE() + (uint32_t)c * 2u, 0) & 1;
        if (upper == 0)
            return c;
    }
    if (U8(W7R_CTYPE() + (((uint32_t)c >> 8) & 0xffu) * 2u, 1) & 0x80) {
        in[0] = (uint8_t)((uint32_t)c >> 8);
        in[1] = (uint8_t)c;
        in[2] = 0;
        n = 2;
    } else {
        in[0] = (uint8_t)c;
        in[1] = 0;
        n = 1;
    }
    got = W7R_FN_101053C0(G32(0x1017e950), 0x100, in, n, out, 3, 0);
    if (got == 0)
        return c;
    if (got == 1)
        return out[0];
    return out[0] | (out[1] << 8);
}

/* 1.2's _strlwr (new in its C library). With a locale handle it maps the string through 0x101053c0 into a
   malloc'ed copy and copies that back; 1.2 passes its changed mapper one more argument (1), which 1.0's has not:
   that path runs only with a locale set, which the game never does. */
// FUNCTION: HGSIM12 0x100ea880
char *__cdecl Crt12_StrLwr(char *s)
{
    char *copy = 0, *p;
    int n;

    if (G32(0x1017e950) == 0) {
        copy = s;
        if (*s != 0) {
            for (p = s; *p != 0; p++) {
#ifdef W7RL_MUTATE /* the checker's negative check only: K..Z kept */
                if (*p > '@' && *p < 'K')
#else
                if (*p > '@' && *p < '[')
#endif
                    *p = (char)(*p + ' ');
            }
            return copy;
        }
        return s;
    }
    n = W7Q_MAP_STRING(G32(0x1017e950), 0x100, (const uint8_t *)s, -1, 0, 0, 0);
    if (n != 0 && (copy = (char *)Heap_Malloc((uint32_t)n)) != 0 &&
        W7Q_MAP_STRING(G32(0x1017e950), 0x100, (const uint8_t *)s, -1, (uint8_t *)copy, n, 0) != 0)
        memcpy(s, copy, strlen(copy) + 1);
    Heap_FreeBlock((uint32_t)(uintptr_t)copy);
    return s;
}
