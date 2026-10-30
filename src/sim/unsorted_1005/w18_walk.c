#include "w18.h"
#include "w2e.h"
#include "../util/access.h"
#include "../util/w3o.h"
#include "../unsorted_100f/w6h.h"
#include "../unsorted_1010/w6k.h"
#include "../unsorted_1010/w7s.h"

#define W18W_NO_PARENT 0xffffffffu

// FUNCTION: HGSIM 0x1005ab44
void __cdecl Sim_1005ab44(uint32_t parent, int32_t *counter, const uint8_t *table, int32_t row, uint8_t *node,
                          int32_t field)
{
    uint16_t flags = 0x100;
    int32_t entry;
    char first;

    if (node == 0)
        return;
    first = (char)U8(node, (uint32_t)field * 0x20u + 0x7c);
    entry = (first == 'g' || first == 'G') ? field + 1 : field;
    if ((int8_t)U8(node, (uint32_t)entry * 0x20u + 0x7c) != 0) {
        const char *name = (const char *)(node + (uint32_t)entry * 0x20u + 0x7c);
        uint32_t index = (uint32_t)*counter;
        float transform[9];
        char *upper;

        transform[0] = transform[1] = transform[2] = 1.0f;
        transform[3] = transform[4] = transform[5] = 0.0f;
        transform[6] = (float)((double)FL(table, (uint32_t)row * 0x48u + 0x24) - FL(node, 0x15c));
        transform[7] = (float)((double)FL(table, (uint32_t)row * 0x48u + 0x28) - FL(node, 0x160));
        transform[8] = (float)((double)FL(table, (uint32_t)row * 0x48u + 0x2c) - FL(node, 0x164));
        upper = Crt_StrUpper(Sim_1010c9f0(name));
        if (Crt_StrStr(upper, (const char *)SIM_PTR(char, 0x10144088)) != 0 ||
            Crt_StrStr(upper, (const char *)SIM_PTR(char, 0x10144090)) != 0 ||
            Crt_StrStr(upper, (const char *)SIM_PTR(char, 0x10144098)) != 0 ||
            Crt_StrStr(upper, (const char *)SIM_PTR(char, 0x101440a0)) != 0)
            flags = (uint16_t)(flags | 0x70);
        Sim_1005aa45(name, (int)index, (int)parent, -1, U16(node, 0x10), flags, transform, (uint32_t)field);
        *counter += 1;
        Sim_1005ab44(parent, counter, table, row + 1, BYTES(U32(node, 0x54)), field);
        Sim_1005ab44(index, counter, node + 0x168, 0, BYTES(U32(node, 0x78)), field);
    } else {
        if ((int8_t)U8(node, 0x7c) != 0) {
            *counter += 1;
            G32(0x101c42a0) += 1;
        }
        Sim_1005ab44(parent, counter, table, row + 1, BYTES(U32(node, 0x54)), field);
        Sim_1005ab44(W18W_NO_PARENT, counter, node + 0x168, 0, BYTES(U32(node, 0x78)), field);
    }
}
