#include "w2j.h"
#include "../util/access.h"
#include "../util/util.h"
#include "../unsorted_1001/w2j.h"

/* Five dwords, front to back like the original's rep movsd. */
static void CopyEntry(uint32_t dst, uint32_t src)
{
    int i;

    for (i = 0; i < 5; i++)
        U32(dst, i * 4) = U32(src, i * 4);
}

static uint32_t Entry(uint32_t obj, int32_t index)
{
    return obj + 0x18 + (uint32_t)index * 0x14u;
}

// FUNCTION: HGSIM 0x1000fd00
int __cdecl Sim_1000fd00(uint32_t unused, void *obj)
{
    uint32_t o = (uint32_t)(uintptr_t)obj;
    int32_t found = 0;
    int32_t n, k, i;

    if (obj == 0)
        return 0;
    n = (int32_t)(Sim_10017892() - 1);
    if (n != 0) {
        k = n + 1;
        CopyEntry(Entry(o, k), Entry(o, 5));
        k++;
        CopyEntry(Entry(o, k), Entry(o, 7));
        S32(o, 0xc) = k + 1;
        for (i = 0; i < n; i++) {
            uint32_t index = (uint32_t)Sim_10015c61(i + 1);

            if (index < G32(0x1015dc64) && S16(PTRTAB(index), 0x1a6) == 0xc) {
                S32(Entry(o, i + 1), 0) = 1;
                found++;
            }
        }
    }
    if (n == 0 || n == found) {
        CopyEntry(Entry(o, 0), o + 0xb8);
        CopyEntry(Entry(o, 1), o + 0xa4);
        S32(o, 0xc) = 2;
    }
    return 1;
}

// FUNCTION: HGSIM 0x1000fe60
int __cdecl Sim_1000fe60(int slot)
{
    int32_t n = (int32_t)(Sim_10017892() - 1);
    int result = !(slot > n && slot != 4);

    if (result) {
        uint32_t index = (uint32_t)Sim_10015c61(slot);

        result = index < G32(0x1015dc64) && S16(PTRTAB(index), 0x1a6) != 0xc;
    }
    return result;
}

// FUNCTION: HGSIM 0x1000ff5b
int __cdecl Sim_1000ff5b(uint32_t unused, void *obj)
{
    uint32_t o = (uint32_t)(uintptr_t)obj;
    int32_t slot, last;

    if (obj == 0)
        return 0;
    slot = S32(o, 8);
    if ((int32_t)(Sim_10017892() - 1) == 0) {
        CopyEntry(Entry(o, 0), o + 0xb8);
        CopyEntry(o + 0x2c, o + 0xa4);
        S32(o, 0xc) = 2;
        return 1;
    }
    if (Sim_1000fe60(slot) == 0) {
        last = S32(o, 0xc);
        CopyEntry(Entry(o, 0), Entry(o, last));
        last = S32(o, 0xc) - 1;
        CopyEntry(Entry(o, 1), Entry(o, last));
        S32(o, 0xc) = 2;
    }
    return 1;
}
