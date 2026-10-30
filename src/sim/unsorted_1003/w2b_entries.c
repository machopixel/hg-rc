#include "w2b.h"
#include "../simmem.h"

#define COUNT SIM_PTR(int32_t, 0x109E4CC0)
#define TABLE SIM_PTR(unsigned char, 0x109E4CC4)
#define ENTRY_SIZE 0x44

// FUNCTION: HGSIM 0x10033c67
unsigned char *__cdecl Sim_10033c67(int value)
{
    int count = *COUNT;
    unsigned char *entry = TABLE;
    int i;

    if (count <= 0)
        return 0;
    for (i = 0; i < count; i++, entry += ENTRY_SIZE) {
        if (Sim_10033580(entry) == value)
            return entry;
    }
    return 0;
}

// FUNCTION: HGSIM 0x10033ced
int __cdecl Sim_10033ced(int value)
{
    int count = *COUNT;
    unsigned char *entry = TABLE;
    int i;

    if (count <= 0)
        return -1;
    for (i = 0; i < count; i++, entry += ENTRY_SIZE) {
        if (Sim_10033580(entry) == value)
            return i;
    }
    return -1;
}
