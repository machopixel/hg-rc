#include "w2e.h"
#include "../unsorted_100f/w2e.h"
#include "../util/access.h"

#define ENTRY_COUNT GS32(0x1015ea80)
#define ENTRY(i) (GBYTES(0x109b9e20) + (uint32_t)(i) * 0x7eu)

// FUNCTION: HGSIM 0x1003aa7d
int __cdecl Sim_1003aa7d(const char *name, uint32_t a1, uint32_t a2, uint32_t a3, uint32_t a4,
                         int a5, int a6, unsigned short a7, uint32_t a8)
{
    int result;
    uint8_t *e;
    int j;

    if (G32(0x1015ea84) == 0)
        Sim_1003ac07();
    if (ENTRY_COUNT >= 0x465)
        return -1;
    if (a4 == 0xFFFFFFFFu)
        a4 = 0xFFFFFFFEu;

    if (a5 == 0) {
        e = ENTRY(ENTRY_COUNT);
        U32(e, 0) = 0xFFFFFFFEu;
        U32(e, 4) = 0xFFFFFFFFu;
        U32(e, 0x48) = a4;
        U32(e, 0x68) = a1;
        U32(e, 0x6c) = a2;
        U32(e, 0x70) = a3;
        Str_CopyN((char *)e + ((uint32_t)a5 << 4) + 8, name, 8);
        U8(e, ((uint32_t)a5 << 4) + 0x10) = 0;
        U32(e, 0x64) = a8;
        result = ENTRY_COUNT;
        ENTRY_COUNT = ENTRY_COUNT + 1;
    } else {
        result = a6;
        if (result < 0)
            return -1;
        e = ENTRY(result);
        Str_CopyN((char *)e + ((uint32_t)a5 << 4) + 8, name, 8);
    }
    U16(e, (uint32_t)a5 * 2u + 0x74) = a7;

    if (a5 < 4) {
        for (j = a5 + 1; j < 4; j++) {
            Str_CopyN((char *)e + ((uint32_t)j << 4) + 8, name, 8);
            U8(e, ((uint32_t)j << 4) + 0x10) = 0;
            U16(e, (uint32_t)j * 2u + 0x74) = a7;
        }
    }
    return result;
}

// FUNCTION: HGSIM 0x1003ac07
void __cdecl Sim_1003ac07(void)
{
    int i, j;

    for (i = 0; i < 0x465; i++) {
        uint8_t *e = ENTRY(i);

        U32(e, 0) = 0xFFFFFFFFu;
        U32(e, 4) = 0xFFFFFFFFu;
        U32(e, 0x48) = 0xFFFFFFFFu;
        U32(e, 0x4c) = 0;
        U16(e, 0x7c) = 0;
        U32(e, 0x70) = 0;
        U32(e, 0x6c) = 0;
        U32(e, 0x68) = 0;
        U32(e, 0x60) = 0;
        for (j = 0; j < 4; j++) {
            U32(e, (uint32_t)j * 4 + 0x50) = 0;
            U8(e, ((uint32_t)j << 4) + 8) = 0;
            U16(e, (uint32_t)j * 2 + 0x74) = 0;
        }
    }
    ENTRY_COUNT = 0;
    G32(0x1015ea84) = 1;
}
