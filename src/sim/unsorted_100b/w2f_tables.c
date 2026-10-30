#include "w2f.h"
#include "../util/access.h"

// FUNCTION: HGSIM 0x100b3f8f
void __cdecl Sim_100b3f8f(void)
{
}

// FUNCTION: HGSIM 0x100b5386
uint32_t __cdecl Sim_100b5386(int i)
{
    uint32_t r = 0;

    if (i < GS32(0x1017d218) && i >= 0)
        r = G32(0x101a212c + (uint32_t)i * 0x88);
    return r;
}

// FUNCTION: HGSIM 0x100b54b3
int __cdecl Sim_100b54b3(int i)
{
    int r = -1;

    if (i <= GS32(0x1017d218) && i >= 0) {
        uint8_t *entry = GBYTES(0x101a2100 + (uint32_t)i * 0x88);

        if ((U32(entry, 0x18) & 0x400) != 0)
            r = S32(entry, 0x24);
    }
    return r;
}

// FUNCTION: HGSIM 0x100b8d5c
int __cdecl Sim_100b8d5c(void)
{
    return -1;
}

// FUNCTION: HGSIM 0x100bd95a
int __cdecl Array_RemoveValue(void *list, uint32_t val)
{
    int found = 0;
    int32_t i;

    (void)*(volatile uint32_t *)BYTES(U32(list, 8)); /* the original loads the first element and ignores it */
    for (i = 0; i < S32(list, 4); i++) {
        if (U32(U32(list, 8), i * 4) == val) {
            U32(U32(list, 8), i * 4) = 0;
            found = 1;
            break;
        }
    }
    if (found) {
        while (i < S32(list, 4) - 1) {
            U32(U32(list, 8), i * 4) = U32(U32(list, 8), i * 4 + 4);
            i++;
        }
        S32(list, 4) = S32(list, 4) - 1;
    }
    return found;
}

// FUNCTION: HGSIM 0x100bda4a
void __cdecl Array_Clear(void *list)
{
    int32_t i;

    if (list != 0)
        S32(list, 4) = 0;
    for (i = 0; i < S32(list, 0); i++)
        U32(U32(list, 8), i * 4) = 0;
}
