#include "w2k.h"
#include "../util/access.h"
#include "../unsorted_100f/w2k.h"

// FUNCTION: HGSIM 0x10107080
uint32_t __cdecl Sim_10107080(unsigned handle)
{
    uint32_t block;

    if (handle >= G32(0x10AC385C))
        return 0;
    block = G32(0x10AC3720u + (uint32_t)((int32_t)handle >> 5) * 4u);
    return U8(block, (handle & 0x1F) * 8 + 4) & 0x40;
}

// FUNCTION: HGSIM 0x101077f0
void *__cdecl Sim_101077f0(uint32_t key)
{
    uint32_t count = G32(0x1015C1C8);
    uint32_t off;

    for (off = 0; off < count * 12u; off += 12) {
        if (G32(0x1015C148 + off + 4) == key)
            return GBYTES(0x1015C148 + off);
    }
    return 0;
}

// FUNCTION: HGSIM 0x1010c940
char *__cdecl Crt_MbsChr(const char *s, unsigned c)
{
    const uint8_t *p = (const uint8_t *)s;
    unsigned ch;

    if (G32(0x1017EBC4) == 0)
        return Crt_StrChr(s, (int)c);
    ch = *p;
    while (ch != 0) {
        if ((GBYTES(0x1017EAC1)[ch & 0xFF] & 4) != 0) {
            unsigned trail = p[1];

            p++;
            if (trail == 0)
                return 0;
            if (c == ((ch << 8) | trail))
                return (char *)(p - 1);
        } else if (c == ch) {
            return (char *)p;
        }
        p++;
        ch = *p;
    }
    return c == 0 ? (char *)p : 0;
}
