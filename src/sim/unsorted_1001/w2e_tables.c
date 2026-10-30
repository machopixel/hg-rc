#include "w2e.h"
#include "../unsorted_100c/w2e.h"
#include "../unsorted_100f/w2e.h"
#include "../util/access.h"

#define TABLE_COUNT GS32(0x1017e5a8)
#define TABLE_ENTRY(i) (GBYTES(0x109a8640) + (uint32_t)(i) * 0x54u)

// FUNCTION: HGSIM 0x1001570f
void __cdecl Sim_1001570f(void *obj)
{
    int i;

    if (TABLE_COUNT == -1)
        return;
    for (i = 0; i < TABLE_COUNT; i++) {
        uint8_t *entry = TABLE_ENTRY(i);

        if ((S16(entry, 0x24) & 1) == 0)
            continue;
        if (U32(entry, 8) != (U32(obj, 4) | 0x200))
            continue;
        if ((U32(obj, 0x1d6) & 0x1000) == 0) {
            if ((U32(obj, 0x1d6) & 0xff) != (uint32_t)i)
                Sim_100ca417(U32(obj, 4), (uint32_t)i | 0x100);
        } else {
            Sim_100ca417(U32(obj, 4), (uint32_t)i | 0x100);
        }
    }
}

// FUNCTION: HGSIM 0x10017852
void __cdecl Sim_10017852(void *obj)
{
    if (U32(obj, 0x1d6) != 0)
        return;
    Sim_100ca417(U32(obj, 4), U32(obj, 0x1d6));
    U32(obj, 0x1d6) = 0x1000;
}

// FUNCTION: HGSIM 0x1001f6c0
void __cdecl Sim_1001f6c0(void)
{
    const uint8_t *src = GBYTES(0x10132248);
    uint8_t *table = GBYTES(0x109f1520);
    unsigned i;

    Mem_Fill(table, -1, 0x2580);
    for (i = 0; i < 0x2c; i++, src += 8) {
        int key = S16(src, 0);

        if (key < 1 || key >= 0x4b0)
            continue;
        U16(table, (uint32_t)key * 8 + 0) = U16(src, 2);
        U16(table, (uint32_t)key * 8 + 2) = U16(src, 4);
        U16(table, (uint32_t)key * 8 + 4) = U16(src, 6);
        U16(table, (uint32_t)key * 8 + 6) = 0;
    }
}
