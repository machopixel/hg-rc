#include <string.h>
#include "w9f.h"
#include "../util/access.h"
#include "../view/view.h"
#include "../view/w2l.h"

// FUNCTION: HGSIM 0x10078a7f
void __cdecl Sim_10078a7f(uint8_t *obj)
{
    uint8_t savedEntry[20], savedRect[20];
    uint32_t pair;
    int32_t *rect;
    uint8_t *entry;

    if (S16(obj, 4) == 0 || U32(obj, 0x30) == 0 || U32(obj, 0x30) == 2 || U32(obj, 0x30) == 1)
        return;
    pair = U32(obj, 0xf4);
    if (pair != 0) {
        if (U32(obj, 0xf8) != 0 && U32(obj, 0xf8) != 3 && U32(obj, 0xf8) != 4)
            Sim_1008f630((void **)(uintptr_t)pair);
        rect = Sim_1008f867(1, (const uint32_t *)(uintptr_t)pair);
        if (rect != 0) {
            entry = GBYTES(0x10185400) + U32(obj, 0xc) * 0x14u;
            memcpy(savedEntry, entry, 20);
            memcpy(savedRect, BYTES(U32(obj, 0xec)), 20);
            memcpy(entry, rect, 20);
            memcpy(BYTES(U32(obj, 0xec)), rect, 20);
            W9F7_CALL_100782a9(obj);
            memcpy(entry, savedEntry, 20);
            memcpy(BYTES(U32(obj, 0xec)), savedRect, 20);
        }
    }
    U32(obj, 0xf8) = G32(0x109e89d8);
}
