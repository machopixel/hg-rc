#include "w5c.h"
#include "../unsorted_100f/w2e.h"
#include "../unsorted_100f/w2k.h"
#include "../util/access.h"

// FUNCTION: HGSIM 0x100474db
char *__cdecl Sim_100474db(const char *name)
{
    char *buf = SIM_PTR(char, 0x1015ef50);
    int i;

    for (i = 0; i < 0x50; i++)
        buf[i] = 0;
    if (*SIM_PTR(char, 0x1015f050) != 0 && Crt_StrChr(name, 0x5c) == 0 && Crt_StrChr(name, 0x2f) == 0)
        SIM_CALL_100fc780(buf, SIM_PTR(char, 0x1013b2e8), SIM_PTR(char, 0x1015f050), name);
    else
        Str_Copy(buf, name);
    return SIM_PTR(char, 0x1015ef50);
}

// FUNCTION: HGSIM 0x1004dcaa
int __cdecl Sim_1004dcaa(int c)
{
    int r;

    U32(G32(0x1015f344), 4) = U32(G32(0x1015f344), 4) - 1;
    if (S32(G32(0x1015f344), 4) >= 0) {
        *(uint8_t *)(uintptr_t)U32(G32(0x1015f344), 0) = (uint8_t)c;
        r = (uint8_t)c;
        U32(G32(0x1015f344), 0) = U32(G32(0x1015f344), 0) + 1;
    } else {
        r = SIM_CALL_100ff030(c & 0xff, (void *)(uintptr_t)G32(0x1015f344));
    }
    return r == -1 ? 2 : 0;
}

// FUNCTION: HGSIM 0x1004e1c6
int __cdecl Sim_1004e1c6(void)
{
    uint8_t c = (uint8_t)(G32(0x1015f340) + (G32(0x1015f348) != 8 ? 1u : 0u));

    if (c == 0)
        return 0;
    if (Sim_1004dcaa(c) != 0)
        return -1;
    if (SIM_CALL_1004db5b(SIM_PTR(char, 0x1015f230), c) != 0)
        return -1;
    G32(0x1015f340) = 0;
    SIM_PTR(uint8_t, 0x1015f230)[G32(0x1015f340)] = 0;
    G32(0x1015f348) = 8;
    return 0;
}

// FUNCTION: HGSIM 0x1004edca
int __cdecl Sim_1004edca(uint32_t a, uint32_t b)
{
    return SIM_CALL_100e85cb(G32(0x109b8be0), a, b);
}

// FUNCTION: HGSIM 0x1004f0d6
void __cdecl Sim_1004f0d6(void *obj)
{
    uint32_t flags;

    if (U32(obj, 4) != G32(0x1017b8c8))
        Str_Copy((char *)obj + 0x98, SIM_PTR(char, 0x1013bab0));
    U32(obj, 0x94) = 0;
    U32(obj, 0x90) = U32(U32(obj, 0x28), 0x5ff);
    flags = SIM_CALL_1004ee50(obj);
    U32(obj, 0x8c) = U32(obj, 0x8c) | flags;
}
