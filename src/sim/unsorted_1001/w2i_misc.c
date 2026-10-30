#include "w2i.h"
#include "../unsorted_1004/fns.h"
#include "../util/access.h"

// FUNCTION: HGSIM 0x10012ad9
uint32_t __cdecl Sim_10012ad9(uint32_t a, uint32_t b, uint32_t c)
{
    (void)c;
    return (a & 0xFFFF0000u) | (b & 0xFFFFu);
}

// FUNCTION: HGSIM 0x10015cf8
void __cdecl Sim_10015cf8(void *obj, int16_t a, int16_t b, uint8_t d)
{
    (void)d;
    if (S16(obj, 0x1a0) == 0) {
        U16(obj, 0x1a0) = (uint16_t)a;
        U16(obj, 0x1a2) = (uint16_t)b;
    }
}

// FUNCTION: HGSIM 0x10015c0e
int __cdecl Sim_10015c0e(void *obj, void *rec)
{
    int16_t index = (int16_t)Sim_10052aaf((int)U32(obj, 8));

    if (index != -1)
        Sim_10015cf8((void *)(uintptr_t)PTRTAB(index), (int16_t)U16(rec, 0), (int16_t)U16(obj, 0xf0), U8(rec, 4));
    return 0;
}

// FUNCTION: HGSIM 0x10019d8c
uint32_t __cdecl Sim_10019d8c(void)
{
    return G32(0x1015D184);
}

// FUNCTION: HGSIM 0x1001c075
void *__cdecl Sim_1001c075(int a, int b, void *tab)
{
    uint32_t entry = 0;
    int i;

    if (tab == 0)
        return 0;
    a = a % b;
    for (i = 0; i < S32(tab, 0); i++) {
        entry = (uint32_t)(uintptr_t)tab + (uint32_t)i * 0x39u + 4u;
        if (a == S32(entry, 0))
            break;
        if (a < S32(entry, 0)) {
            entry = (uint32_t)(uintptr_t)tab + (uint32_t)((i + S32(tab, 0) - 1) % S32(tab, 0)) * 0x39u + 4u;
            break;
        }
    }
    return (void *)(uintptr_t)entry;
}

// FUNCTION: HGSIM 0x1001db13
int __cdecl Sim_1001db13(void *tab, int index)
{
    uint32_t entry;

    if (tab == 0 || index >= S32(tab, 0) || index < -1)
        return -1;
    do {
        index = (int)((uint32_t)index + 1u) % S32(tab, 0);
        entry = (uint32_t)(uintptr_t)tab + (uint32_t)index * 0x39u + 4u;
    } while (U32(entry, 0xd) != 0 && U32(entry, 0xd) != 1);
    return index;
}
