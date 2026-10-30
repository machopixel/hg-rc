#include "w9b.h"
#include "w5c.h"
#include "../util/access.h"

// FUNCTION: HGSIM 0x1001b902
uint8_t *__cdecl Anim_RecordOfId(int id)
{
    int row = id & 0xff;
    int group = (id & 0xf00) >> 8;
    int table = id & 0xff000;
    uint8_t *result = 0;
    int i;

    switch (table) {
    case 0:
        i = row + group * 0x1a;
        if (i >= 0 && i < 0x4e)
            result = GBYTES(0x109f4240) + (uint32_t)i * 0x18;
        break;
    case 0x1000:
        i = row + group * 0x1a;
        if (i >= 0 && i < 0x4e)
            result = GBYTES(0x10a37b80) + (uint32_t)i * 0x18;
        break;
    case 0x2000:
        switch (row) {
        case 0:    result = GBYTES(0x10a1e340); break;
        case 4:    result = GBYTES(0x10a1e3b8); break;
        case 5:    result = GBYTES(0x10a1e3d0); break;
        case 9:    result = GBYTES(0x10a1e3a0); break;
        case 0xa:  result = GBYTES(0x10a1e358); break;
        case 0xe:  result = GBYTES(0x10a1e370); break;
        case 0x12: result = GBYTES(0x10a1e388); break;
        default: break;
        }
        break;
    case 0x3000:
        switch (row) {
        case 0:    result = GBYTES(0x10a1e430); break;
        case 4:    result = GBYTES(0x10a1e490); break;
        case 5:    result = GBYTES(0x10a1e4a8); break;
        case 0xa:  result = GBYTES(0x10a1e448); break;
        case 0xe:  result = GBYTES(0x10a1e460); break;
        case 0x12: result = GBYTES(0x10a1e478); break;
        default: break;
        }
        break;
    case 0x4000:
        result = GBYTES(0x10a382e0);
        break;
    case 0x5000:
        result = GBYTES(0x10a382f8);
        break;
    default:
        break;
    }
    return result;
}

/* One registration: the name is the text at `name` (an address of the DLL's data). */
struct W9bAnimName {
    uint32_t name;
    int16_t id;
    int8_t reload;
};

/* The 22 ids of each of the three gear groups (0x000, 0x100, 0x200), in the original's order. */
static const int16_t W9b_GearIds[22] = {
    0, 1, 2, 3, 4, 5, 8, 9, 0xa, 0xe, 0xb, 0xc, 0xf, 0x10, 0xd, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x19,
};

static const struct W9bAnimName W9b_Stand3[6] = {
    {0x10131c84, 0x3000, 0}, {0x10131c90, 0x300a, 0}, {0x10131c9c, 0x300e, 0},
    {0x10131ca8, 0x3012, 0}, {0x10131cb4, 0x3004, 0}, {0x10131cc0, 0x3005, 0},
};

static const struct W9bAnimName W9b_Stand2[7] = {
    {0x10131ccc, 0x2000, 0}, {0x10131cd8, 0x200a, 0}, {0x10131ce4, 0x200e, 0}, {0x10131cf0, 0x2012, 0},
    {0x10131cfc, 0x2009, 0}, {0x10131d08, 0x2004, 0}, {0x10131d14, 0x2005, 0},
};

/* The 16 ids of each of the three 0x1xxx groups (0x1000, 0x1100, 0x1200). */
static const int16_t W9b_OtherIds[16] = {
    0, 1, 2, 3, 4, 5, 0xa, 0xe, 0xb, 0xc, 0xf, 0x10, 0x12, 0x13, 0x14, 0x18,
};

static void W9b_Register(const struct W9bAnimName *list, int count)
{
    int i;

    for (i = 0; i < count; i++)
        Sim_1001bb1f(SIM_PTR(const char, list[i].name), list[i].id, list[i].reload);
}

// FUNCTION: HGSIM 0x1001d0fa
void __cdecl Anim_RegisterNames(void)
{
    int g, i;

    if (G32(0x1017e800) != 0) {
        /* names 0x10131954.. in steps of 12; ids 4 and 5 of each group without reload */
        for (g = 0; g < 3; g++) {
            for (i = 0; i < 22; i++) {
                int id = W9b_GearIds[i];

                Sim_1001bb1f(SIM_PTR(const char, 0x10131954 + (uint32_t)(g * 22 + i) * 12), (g << 8) | id,
                             (id == 4 || id == 5) ? 0 : 1);
            }
        }
    }
    if (G32(0x1017e7ec) != 0)
        Sim_1001bb1f(SIM_PTR(const char, 0x10131c6c), 0x4017, 0);
    if (G32(0x1017e7f0) != 0)
        Sim_1001bb1f(SIM_PTR(const char, 0x10131c78), 0x5017, 0);
    if (G32(0x1017e7f8) != 0)
        W9b_Register(W9b_Stand3, 6);
    if (G32(0x1017e7fc) != 0)
        W9b_Register(W9b_Stand2, 7);
    if (G32(0x1017e7f4) != 0) {
        /* names 0x10131d20.. in steps of 12 */
        for (g = 0; g < 3; g++) {
            for (i = 0; i < 16; i++) {
                int id = W9b_OtherIds[i];

                Sim_1001bb1f(SIM_PTR(const char, 0x10131d20 + (uint32_t)(g * 16 + i) * 12), 0x1000 | (g << 8) | id,
                             (id == 4 || id == 5) ? 0 : 1);
            }
        }
    }
}
