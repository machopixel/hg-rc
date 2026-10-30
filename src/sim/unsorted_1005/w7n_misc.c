#include "w7n.h"
#include "w2e.h"
#include "../util/w3o.h"

// FUNCTION: HGSIM 0x1005a91a
void __cdecl Sim_1005a91a(void *obj, int index, int value)
{
    float transform[9];
    uint8_t *table;
    char first;
    int entry;
    int next = value;
    int current;

    transform[0] = transform[1] = transform[2] = 1.0f;
    transform[3] = transform[4] = transform[5] = 0.0f;
    transform[6] = -FL(U32(obj, 0x88), 0x15c);
    transform[7] = -FL(U32(obj, 0x88), 0x160);
    transform[8] = -FL(U32(obj, 0x88), 0x164);
    table = BYTES(U32(obj, 0x88));
    first = (char)U8(table, index * 0x20 + 0x7c);
    entry = (first == 'g' || first == 'G') ? index + 1 : index;
    Sim_1005aa45((const char *)(BYTES(U32(obj, 0x88)) + entry * 0x20 + 0x7c), next, -1, -1,
                 U16(U32(obj, 0x88), 0x10), 0x100, transform, (uint32_t)index);
    current = next;
    next++;
    W7N_CALL_1005ab44(current, &next, BYTES(U32(obj, 0x88)) + 0x168, 0, U32(U32(obj, 0x88), 0x78), index);
}
