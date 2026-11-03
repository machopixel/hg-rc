#include "w7m.h"
#include "../util/access.h"
#include "../unsorted_100f/w2e.h"
#include "../unsorted_100f/w6h.h"

static void Column(uint32_t surface, int32_t x, int32_t y, int32_t top, int32_t colour)
{
    W7M_CALL_100ef727(surface, x, y, x, top, 0, SIM_PTR(u16a, 0x101659e0)[colour]);
}

// FUNCTION: HGSIM 0x10051a36
void __cdecl Sim_10051a36(uint32_t surface, int32_t x, int32_t y, int32_t len, int32_t height, int32_t colour)
{
    int32_t half = len / 2;
    int32_t end = half / 2;
    int32_t top = (int32_t)((uint32_t)y - (uint32_t)height);
    int32_t dark = (int32_t)((uint32_t)colour - 1);
    int32_t darker = (int32_t)((uint32_t)colour - 2);
    int32_t i;

    if (height <= 0)
        return;
    for (i = 0; i < end; i++)
        Column(surface, (int32_t)((uint32_t)x + (uint32_t)i), y, top, dark);
    for (i = end; i < half; i++)
        Column(surface, (int32_t)((uint32_t)x + (uint32_t)i), y, top, colour);
    end = half + ((int32_t)((uint32_t)len - (uint32_t)half)) / 2;
    for (i = half; i < end; i++)
        Column(surface, (int32_t)((uint32_t)x + (uint32_t)i), y, top, dark);
    for (i = end; i < len; i++)
        Column(surface, (int32_t)((uint32_t)x + (uint32_t)i), y, top, darker);
}

static int IsSpace(char c)
{
    if (GS32(0x1015965c) > 1)
        return Crt_IsCType(c, 8);
    return *(u16a *)(uintptr_t)(G32(0x10159450) + (uint32_t)(int32_t)c * 2u) & 8;
}

// FUNCTION: HGSIM 0x1005346f
char *__cdecl Str_TrimSpace(char *s)
{
    int32_t n, last;

    if (s == 0)
        return s;
    while (IsSpace(*s))
        s++;
    n = (int32_t)Str_Length(s);
    for (;;) {
        last = n;
        n--;
        if (last <= 0 || !IsSpace(s[n]))
            break;
    }
    s[n + 1] = 0;
    return s;
}
