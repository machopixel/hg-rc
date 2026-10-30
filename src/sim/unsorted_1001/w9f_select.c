#include "w9f.h"
#include "w7l.h"
#include "w8t.h"
#include "../util/util.h"
#include "../util/access.h"
#include "../unsorted_1002/w8t.h"
#include "../unsorted_1004/fns.h"
#include "../unsorted_1004/w3p.h"

// FUNCTION: HGSIM 0x10016117
int __cdecl Sim_10016117(int value, int16_t kind)
{
    int16_t after = -1;
    uint8_t *cur = BYTES(PTRTAB(G32(0x1017b8c8)));
    int idx = Sim_10015c61(value);
    int16_t sel;
    int16_t word;
    int32_t flags;
    uint8_t *found;

    if (idx == -1)
        return 0;
    sel = 0x1000;
    switch (kind) {
    case 2:
        if (U32(cur, 0xf0) != 0)
            sel = (int16_t)U16(cur, 0xf0);
        if (sel == -1) {
            Sim_10020256(0xc, value);
            sel = 0x1000;
        } else if ((sel & 0x200) != 0 && ((sel & 0xff) == idx || Sim_10052be7(sel & 0xff) == 0)) {
            Sim_10020256(0xc, value);
            sel = 0x1000;
        } else if ((sel & 0x100) != 0) {
            sel = -1;
            Sim_10020256(0xc, value);
        } else {
            after = 1;
        }
        break;
    case 3:
        if (U32(cur, 0xf0) != 0)
            sel = (int16_t)U16(cur, 0xf0);
        if (sel <= 0 || ((sel & 0x200) != 0 && (sel & 0xff) == idx)) {
            sel = 0x1000;
            Sim_10020256(0xc, value);
        } else {
            sel = 0;
        }
        break;
    case 5:
        sel = 0x2201;
        after = 2;
        break;
    case 7:
        if (U32(cur, 0xf0) != 0)
            sel = (int16_t)U16(cur, 0xf0);
        if (sel <= 0 || ((sel & 0x200) != 0 && (sel & 0xff) == idx)) {
            sel = 0x1000;
            Sim_10020256(0xc, value);
        } else {
            after = 3;
        }
        break;
    case 8:
        sel = 0x2100;
        break;
    case 11:
        sel = 0;
        break;
    case 13:
        word = (int16_t)U16(cur, 0xf0);
        if (word > 0 && (word & 0x200) != 0 && (word & 0xff) != (int32_t)G32(0x1017b8c8)) {
            sel = word;
            Sim_10020256(8, value);
        } else {
            sel = 0x1000;
            Sim_10020256(0xc, value);
        }
        break;
    case 14:
        flags = S32(cur, 0x118);
        if (flags > 0 && (flags & 0x100) != 0) {
            sel = (int16_t)flags;
            Sim_10020256(0xa, value);
        } else {
            sel = 0x1000;
            Sim_10020256(0xc, value);
        }
        break;
    case 15:
        word = Sim_10015f5b(cur, BYTES(PTRTAB(idx)));
        if (word == 0x1000) {
            sel = 0x1000;
            Sim_10020256(0xb, value);
        } else {
            sel = word;
            Sim_10020256(9, value);
        }
        break;
    default:
        break;
    }
    if ((sel & 0x1000) != 0)
        return 0;
    found = BYTES(PTRTAB(idx & 0xff));
    /*
     * The original pushes `kind` and `sel` as 16-bit values in registers whose upper halves hold leftovers; the
     * callees read only the low 16 bits (Sim_10040f5f casts to uint16_t / uint8_t, Sim_10015d30 takes int16_t),
     * so clean values are passed here.
     */
    if (U32(found, 0x14) == 3 && U32(found, 0xc) != G32(0x1017b8c8)) {
        Sim_10040f5f(cur, idx | 0x200, (int)(uint16_t)kind, (int)(uint16_t)sel);
        return 1;
    }
    if (idx == (int32_t)G32(0x1017b8c8)) {
        Sim_10015d30(cur, 0x4200, kind, sel);
        if (G32(0x1015ec58) != 0)
            Sim_10040f5f(cur, 0x4200, (int)(uint16_t)kind, (int)(uint16_t)sel);
    } else {
        Sim_10015d30(cur, (int16_t)(idx | 0x200), kind, sel);
    }
    if (after != -1)
        Sim_10020256(after, value);
    return 1;
}
