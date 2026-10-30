#include "w2e.h"
#include "../unsorted_100f/w2e.h"
#include "../util/access.h"

#define TABLE_COUNT GS32(0x1017e5a8)
#define TABLE_ENTRY(i) (GBYTES(0x109a8640) + (uint32_t)(i) * 0x54u)

// FUNCTION: HGSIM 0x100c4699
int __cdecl Sim_100c4699(void *obj, uint32_t *out)
{
    int found = 0;
    int count = GS32(0x1015dc64);
    int i;

    if (obj == 0) {
        *out = 0;
        return 0;
    }
    {
        uint32_t *w = (uint32_t *)GBYTES(0x1017e240);
        uint32_t key = U32(obj, 4);

        Mem_Fill(w, 0, 0x64);
        for (i = 0; i < count; i++) {
            uint8_t *rec = BYTES(PTRTAB(i));
            uint32_t flags = U16(rec, 0x1a8);

            if ((flags & 0x200) != 0 && (flags & 0xff) == key) {
                found++;
                *w++ = U32(rec, 4);
            }
        }
    }
    *out = (uint32_t)(uintptr_t)GBYTES(0x1017e240);
    return found;
}

// FUNCTION: HGSIM 0x100ca417
int __cdecl Sim_100ca417(uint32_t a, uint32_t b)
{
    int index;
    int i;

    if ((b & 0x100) == 0)
        return -1;
    index = (int)(b & 0xff);
    if (index >= TABLE_COUNT)
        return -1;
    if ((S16(TABLE_ENTRY(index), 0x24) & 1) == 0 || (a | 0x200) != U32(TABLE_ENTRY(index), 8))
        return -1;

    for (i = index; i < TABLE_COUNT - 1; i++)
        Mem_Move(TABLE_ENTRY(i), TABLE_ENTRY(i + 1), 0x54);

    for (i = 0; i < GS32(0x1015dc64); i++) {
        uint8_t *rec = BYTES(PTRTAB(i));

        if (rec == 0)
            continue;
        if ((U16(rec, 0x1a8) & 0x100) != 0 && (int)(U16(rec, 0x1a8) & 0xff) > index)
            U16(rec, 0x1a8) = (uint16_t)(U16(rec, 0x1a8) - 1);
        if ((U16(rec, 0x1aa) & 0x100) != 0 && (int)(U16(rec, 0x1aa) & 0xff) > index)
            U16(rec, 0x1aa) = (uint16_t)(U16(rec, 0x1aa) - 1);
        if ((U32(rec, 0xf0) & 0x100) != 0 && (int)(U32(rec, 0xf0) & 0xff) > index && U32(rec, 4) != G32(0x1017b8c8))
            U32(rec, 0xf0) = U32(rec, 0xf0) - 1;
        if ((U32(rec, 0x118) & 0x100) != 0 && (int)(U32(rec, 0x118) & 0xff) > index && U32(rec, 4) != G32(0x1017b8c8))
            U32(rec, 0x118) = U32(rec, 0x118) - 1;
        if ((U32(rec, 0x1d6) & 0x100) != 0 && (int)(U32(rec, 0x1d6) & 0xff) > index)
            U32(rec, 0x1d6) = U32(rec, 0x1d6) - 1;
    }
    TABLE_COUNT = TABLE_COUNT - 1;
    return TABLE_COUNT;
}
