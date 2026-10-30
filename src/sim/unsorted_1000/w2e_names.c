#include "w2e.h"
#include "../unsorted_1001/w2e.h"
#include "../unsorted_100f/w2e.h"
#include "../util/access.h"

#define NAME_COUNT GS32(0x10a38de0)
#define NAME_ENTRY(i) (GBYTES(0x10a39200) + (uint32_t)(i) * 0x14u)

// FUNCTION: HGSIM 0x1000df9c
int __cdecl Sim_1000df9c(const char *name)
{
    int free_slot = -1;
    int i;

    for (i = 1; i < NAME_COUNT + 2; i++) {
        if (Str_Compare(name, (const char *)NAME_ENTRY(i) + 4) == 0) {
            U32(NAME_ENTRY(i), 0) = U32(NAME_ENTRY(i), 0) + 1;
            return i;
        }
        if (free_slot == -1 && U32(NAME_ENTRY(i), 0) == 0)
            free_slot = i;
    }
    NAME_COUNT = NAME_COUNT + 1;
    Str_Copy((char *)NAME_ENTRY(free_slot) + 4, name);
    U32(NAME_ENTRY(free_slot), 0) = 1;
    return free_slot;
}

// FUNCTION: HGSIM 0x1000ecb0
void __cdecl Sim_1000ecb0(const char *name)
{
    int index;
    int i;

    if (G32(0x1015d0a4) == 0)
        return;
    index = Sim_1000df9c(name);
    for (i = 0; i < 0x10; i++) {
        uint8_t *block = BYTES(G32(0x1015d0a4));

        if (U32(block, (uint32_t)i * 4 + 0x54) != 0 && S32(block, (uint32_t)i * 4 + 0x2d4) == index) {
            U32(block, (uint32_t)i * 4 + 0x54) = 3;
            break;
        }
    }
}

// FUNCTION: HGSIM 0x1000fee8
int __cdecl Sim_1000fee8(uint32_t a, void *b)
{
    (void)a;
    if (b == 0)
        return 0;
    Sim_10010183(b, BYTES(U32(b, 0x24)));
    Sim_10010267(b, BYTES(U32(b, 0x4c)));
    Sim_1001034b(b, BYTES(U32(b, 0x74)));
    return 1;
}
