#include "w8u.h"
#include "fns.h"
#include "w2h.h"
#include "w2j.h"
#include "w4u.h"
#include "w7m.h"
#include "../unsorted_1000/w2i.h"
#include "../unsorted_1000/w5a.h"
#include "../unsorted_1002/w2h.h"
#include "../unsorted_1004/w3p.h"
#include "../unsorted_100a/w5c.h"
#include "../unsorted_100a/w5x.h"
#include "../unsorted_100a/w6g.h"
#include "../view/w2d.h"
#include "../view/w2l.h"
#include "../win/w4v.h"
#include "../util/util.h"
#include "../util/access.h"

// FUNCTION: HGSIM 0x10038913
int __cdecl Sim_10038913(uint8_t *rec, int32_t key, int skip)
{
    uint32_t data;      /* [ebp-0x14] */
    uint32_t node = 0;  /* [ebp-8] */
    uint32_t part = 0;  /* [ebp-4] */
    int32_t slot;

    if (U32(rec, 0xc45) != 0)
        return 0;
    Sim_10038dd3(&data, &node, &part, (uint32_t *)rec, key, skip);
    if (part == 0)
        return 0;
    node = (uint32_t)(uintptr_t)Sim_100aae0f((void *)(uintptr_t)U32(U32(rec, 0), 0x48), SIM_PTR(const char, 0x1013a544));   /* "RAHN" */
    if (node == 0)
        return 0;
    Sim_1003913b(part, rec);
    Sim_100aa351((void *)(uintptr_t)U32(part, 4));
    Sim_100ab186((void *)(uintptr_t)node, (void *)(uintptr_t)part);
    Sim_10009408((void *)(uintptr_t)U32(part, 0x8c), 0x100);
    Sim_1000942e((void *)(uintptr_t)U32(part, 0x8c), (int16_t)U16(U32(rec, 0), 4));
    slot = Sim_1003b485(part);
    if (slot >= 0) {
        Sim_1003b449(slot, U32(U32(rec, 0), 4));
        Sim_1003b45d(slot, (uint32_t)Sim_1003b485(U32(part, 0)));
        W8U_CALL_1003aecb(slot, 0, (void *)(uintptr_t)part);
    }
    Sim_1000ebfb(SIM_PTR(const char, 0x1013a54c), 0, 0, 5, 0x50);     /* "AFXHDWPS" */
    if (G32(0x1015ec58) != 0 && U32(U32(rec, 0), 4) == G32(0x1017b8c8))
        Sim_10041203((int)U32(U32(rec, 0), 4), U32(data, 0x118));
    return 1;
}

// FUNCTION: HGSIM 0x1003a990
int __cdecl Sim_1003a990(uint8_t *unit)
{
    uint32_t count = 0;
    uint8_t *block;
    int i, slot;

    for (i = 0; i < GS32(0x1015ea80); i++)
        if (U32(GBYTES(0x109b9e20), 0x7e * i) == U32(unit, 4))
            count++;
    block = Heap_AllocRaw((count * W8U_CALL_100ab1f2_ARG(G32(0x10134f88))) & 0xffff);
    if (block == 0)
        return 0;
    for (i = 0; i < GS32(0x1015ea80); i++) {
        if (U32(GBYTES(0x109b9e20), 0x7e * i) != U32(unit, 4))
            continue;
        for (slot = 3; slot > -1; slot--)
            W8U_CALL_1003aecb(i, slot, block);
        block += W8U_CALL_100ab1f2();
    }
    return 1;
}

// FUNCTION: HGSIM 0x1003ad51
int __cdecl Gear_SetLod(uint32_t unit, int slot)
{
    int result = 1;
    uint8_t *e;
    uint8_t *obj;
    int i, k;

    if (slot == -1)
        return 1;
    G32(0x1017ed80) = 1;
    G32(0x1017ed84) = 0x100;
    G32(0x1017ed88) = unit;
    for (i = 0; i < GS32(0x1015ea80); i++) {
        uint8_t *item;

        e = GBYTES(0x109b9e20 + 0x7e * i);
        if (U32(e, 0) != unit)
            continue;
        if ((S16(PTRTAB(U32(e, 0)), 0x1c) & 2) != 0 && U32(e, 0x4c) == 0) {
            for (k = 0; k < 4; k++)
                if (U32(e, 0x50 + 4 * k) != 0)
                    Sim_1008a40a((void *)(uintptr_t)U32(e, 0x50 + 4 * k));
        } else if (W8U_CALL_1003aecb(i, slot, 0) == 0) {
            result = 0;
        }
        item = BYTES(U32(e, 0x50 + 4 * slot));
        if (item != 0) {
            uint32_t view = U32(item, 0x28);

            if (U32(e, 0x4c) == 0 && U32(view, 0) != 0)
                Sim_1008a37f(item);
        }
    }
    obj = BYTES(U32(PTRTAB(unit), 0x48));
    if (obj != 0 && U32(obj, 0x8c) != 0)
        W8U_CALL_100897c7(U32(obj, 0x8c));
    G32(0x1017ed80) = 0;
    return result;
}
