#include "fns.h"
#include "../simmem.h"

/* Entries are 0x7e bytes, so fields are not always 4-byte aligned. */
#define COUNT SIM_PTR(int32_t, 0x1015EA80)
#define TABLE SIM_PTR(unsigned char, 0x109B9E20)
#define FIELD(p, off) (*(uint32_t *)((unsigned char *)(p) + (off)))
#define ENTRY(i) (TABLE + (uint32_t)(i) * 0x7eu)

/* Entries whose first dword is -2 take the dword at src+4; their +0x48 goes from -1 to -2. */
// FUNCTION: HGSIM 0x1003acf3
void __cdecl Sim_1003acf3(const unsigned char *src)
{
    int i;

    for (i = 0; i < *COUNT; i++) {
        unsigned char *e = ENTRY(i);

        if (FIELD(e, 0) == 0xFFFFFFFEu) {
            FIELD(e, 0) = FIELD(src, 4);
            if (FIELD(e, 0x48) == 0xFFFFFFFFu)
                FIELD(e, 0x48) = 0xFFFFFFFEu;
        }
    }
}

// FUNCTION: HGSIM 0x1003b417
uint32_t __cdecl Sim_1003b417(int index)
{
    uint32_t result = 0;

    if (index < *COUNT && index >= 0)
        result = FIELD(ENTRY(index), 0x60);
    return result;
}

// FUNCTION: HGSIM 0x1003b485
int __cdecl Sim_1003b485(uint32_t key)
{
    int i;

    for (i = 0; i < *COUNT; i++) {
        if (FIELD(ENTRY(i), 0x60) == key)
            return i;
    }
    return -1;
}

// FUNCTION: HGSIM 0x1003b50f
void __cdecl Sim_1003b50f(int index, uint32_t value)
{
    FIELD(ENTRY(index), 0x4c) = value & 0xFFFFu;
}
