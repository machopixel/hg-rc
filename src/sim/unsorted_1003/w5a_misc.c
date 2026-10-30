#include "w5a.h"
#include "w2e.h"
#include "../unsorted_1000/w5a.h"
#include "../unsorted_1007/w2e.h"
#include "../unsorted_1007/w5a.h"
#include "../unsorted_100f/w2e.h"
#include "../util/access.h"
#include "../win/w4u_api.h"

// FUNCTION: HGSIM 0x10031917
void __cdecl Sim_10031917(void *p)
{
    int i;

    if (p == 0)
        return;
    if (U32(p, 0x18) == 0) {
        for (i = 0; i < 4; i++)
            Sim_10031917((void *)(uintptr_t)U32(p, 0x1c + i * 4));
    }
    W4u_HeapFree((uint32_t)(uintptr_t)p);
}

// FUNCTION: HGSIM 0x1003462f
void __cdecl Sim_1003462f(const char *s)
{
    const uint8_t *src = (const uint8_t *)s;
    uint8_t *dst = (uint8_t *)(uintptr_t)0xB0F00;
    int len = (int)Str_Length(s);
    int i;

    if (len > 0x50)
        len = 0x50;
    W5A_CALL_100345e0();
    W5A_CALL_100345fc();
    for (i = 0; i < len; i++) {
        *dst = *src;
        dst += 2;
        src++;
    }
}

// FUNCTION: HGSIM 0x1003ce00
void __cdecl Sim_1003ce00(uint32_t a, uint32_t b, int c, uint32_t d, uint32_t e, void *obj)
{
    int isMinus31;
    int isOwner;
    uint32_t saved;

    if (obj == 0)
        return;
    isMinus31 = (c == (int)(G32(0x1015dc18) - 0x31u));
    isOwner = (c == S32(obj, 0x10));
    if (!((isOwner && G32(0x1015dc18) == 0xd) || isMinus31))
        return;
    U8(G32(0x1017b8c4), 1) = 0;
    saved = G32(0x1015ebbc);
    G32(0x1015ebbc) = 0;
    W5A_CALL_1004954e(0x76);
    G32(0x1015ebbc) = saved;
    Sim_10074c8f();
}

// FUNCTION: HGSIM 0x1003ff72
void __cdecl Sim_1003ff72(void *obj, int idx)
{
    char text[0x64];

    if (PTRTAB(idx) == 0)
        return;
    U8(obj, 0x51) = 0;
    W5A_SPRINTF(text, SIM_PTR(char, 0x1013ad50), (char *)(uintptr_t)(PTRTAB(idx) + 0x124), (char *)obj + 2);
    Sim_1007df6c(text, (const char *)(uintptr_t)G32(0x109b98c0), 0x43e, 0x32);
    Sim_1000ebfb(SIM_PTR(char, 0x1013ad58), 0, 0, 5, 0x32);
}
