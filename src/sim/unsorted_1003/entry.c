#include "fns.h"
#include "../simmem.h"

#define FIELD(p, off) (*(uint32_t *)((unsigned char *)(p) + (off)))
#define COUNT SIM_PTR(int32_t, 0x109E4CC0)
#define TABLE SIM_PTR(unsigned char, 0x109E4CC4)

// FUNCTION: HGSIM 0x1003356a
int __cdecl Sim_1003356a(const unsigned char *entry)
{
    return entry ? (int)FIELD(entry, 0x10) : -1;
}

// FUNCTION: HGSIM 0x10033580
int __cdecl Sim_10033580(const unsigned char *entry)
{
    if (entry && FIELD(entry, 0x14) != 0xFFFFFFFFu)
        return (int)FIELD(entry, 0x14);
    return -1;
}

/* Marks the entry as in use (+0x10 = 1) with the value at src+4 in +0x14. */
// FUNCTION: HGSIM 0x1003359f
int __cdecl Sim_1003359f(unsigned char *entry, const unsigned char *src)
{
    if (entry == 0 || src == 0)
        return 0;
    FIELD(entry, 0x14) = FIELD(src, 4);
    FIELD(entry, 0x10) = 1;
    return 1;
}

/* Clears the entry (+0x10 = 0, +0x14 = -1); src is only checked for non-null. */
// FUNCTION: HGSIM 0x100335cf
int __cdecl Sim_100335cf(unsigned char *entry, const unsigned char *src)
{
    if (entry == 0 || src == 0)
        return 0;
    FIELD(entry, 0x14) = 0xFFFFFFFFu;
    FIELD(entry, 0x10) = 0;
    return 1;
}

// FUNCTION: HGSIM 0x10033c42
unsigned char *__cdecl Sim_10033c42(int index)
{
    if (index < 0 || index >= *COUNT)
        return 0;
    return TABLE + (uint32_t)index * 0x44u;
}
