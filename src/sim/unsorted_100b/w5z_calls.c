#include "w5z.h"
#include "../simmem.h"
#include "../util/access.h"
#include "w4v.h"
#include "../unsorted_1000/w2e.h"

typedef void(__cdecl *Fn100b469d)(void);
typedef void(__cdecl *Fn1003b534)(void);
typedef void(__cdecl *Fn100bc280)(uint32_t a, uint32_t b);
typedef void(__cdecl *Fn100b48de)(int index, uint32_t value);
typedef uint8_t *(__cdecl *Fn10100620)(const void *key, uint32_t base, uint32_t count, uint32_t size, const void *compare);
typedef void(__cdecl *Fn100fc780)(char *buf, const char *format, uint32_t a, uint32_t b);
typedef void(__cdecl *Fn1005f9aa)(char *buf);

// FUNCTION: HGSIM 0x100b388a
void __cdecl Sim_100b388a(void)
{
    if (G32(0x1019a07c) != 0) {
        G32(0x1017d228) = 1;
        SIM_FN(Fn100b469d, 0x100b469d)();
        G32(0x1019a07c) = 0;
        G32(0x1017d228) = G32(0x1019a07c);
    }
    SIM_FN(Fn1003b534, 0x1003b534)();
    Sim_100b4213();
}

// FUNCTION: HGSIM 0x100b3f94
void __cdecl Sim_100b3f94(uint32_t unused)
{
    (void)unused;
    G32(0x1017d224) = G32(0x1017d224) - 1;
    if (GS32(0x1017d224) > -1)
        G32(0x101507f8) = SIM_PTR(u32a, 0x101c4220)[GS32(0x1017d224)];
    else
        SIM_FN(Fn100bc280, 0x100bc280)(0x22, 0);
}

// FUNCTION: HGSIM 0x100b418e
void __cdecl Sim_100b418e(void)
{
    int i;

    for (i = 0; i < GS32(0x1017d218); i++)
        SIM_FN(Fn100b48de, 0x100b48de)(i, U32(GBYTES(0x101a2100u + (uint32_t)i * 0x88u), 0x10));
}

// FUNCTION: HGSIM 0x100b787f
uint32_t __cdecl Sim_100b787f(const void *table, uint32_t key)
{
    uint32_t result = key;
    uint32_t search_key = key;

    if (table != 0) {
        uint8_t *found = SIM_FN(Fn10100620, 0x10100620)(&search_key, U32(table, 4), U32(table, 0), 8, SIM_PTR(void, 0x100b6b87));

        if (found != 0)
            result = U32(found, 4);
    }
    return result;
}

// FUNCTION: HGSIM 0x100bc4f7
char *__cdecl Sim_100bc4f7(uint32_t a, uint32_t b)
{
    char *buf = SIM_PTR(char, 0x1017da18);

    SIM_FN(Fn100fc780, 0x100fc780)(buf, SIM_PTR(const char, 0x10151484), a, b);
    SIM_FN(Fn1005f9aa, 0x1005f9aa)(buf);
    Mono_PrintText(buf);
    return buf;
}
