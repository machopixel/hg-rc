#include "w3o.h"
#include "../unsorted_100c/w3o.h"
#include "../unsorted_100f/w2e.h"
#include "../util/w3o.h"

// FUNCTION: HGSIM 0x100b42d5
void __cdecl Sim_100b42d5(int n)
{
    Sim_100cc887(GBYTES(0x101a2100u + (uint32_t)n * 0x88 + 0x84));
}

// FUNCTION: HGSIM 0x100bda9f
uint32_t __cdecl Sim_100bda9f(void *table, uint32_t key, int(__cdecl *cmp)(uint32_t element, uint32_t key))
{
    int lo, hi;

    if (U32(table, 4) == 0)
        return 0;
    lo = 0;
    hi = (int)U32(table, 4) - 1;
    while (lo <= hi) {
        int mid = (hi + lo) >> 1;
        int result = cmp(G32(U32(table, 8) + (uint32_t)mid * 4), key);

        if (result == 0)
            return G32(U32(table, 8) + (uint32_t)mid * 4);
        if (result < 0)
            lo = mid + 1;
        else
            hi = mid - 1;
    }
    return 0;
}

// FUNCTION: HGSIM 0x100baf3f
uint32_t __cdecl Sim_100baf3f(uint32_t *key, uint32_t arg, uint32_t *out)
{
    uint32_t table = Sim_100bda9f((void *)(uintptr_t)G32(0x1017d604), arg,
                                  SIM_FN(int(__cdecl *)(uint32_t, uint32_t), 0x100b971d));
    int32_t bucket;
    uint32_t entry;

    if (table == 0)
        return 0;
    bucket = (int32_t)(key[0] ^ key[1]) % (int32_t)U32(table, 0);
    for (entry = G32(U32(table, 0x10) + (uint32_t)bucket * 4); entry != 0; entry = U32(entry, 0x1c)) {
        if (Str_CompareN((const char *)(uintptr_t)entry, (const char *)key, 8) == 0) {
            uint64_t stamp;

            U32(entry, 8) = U32(entry, 8) + 1;
            stamp = ((uint64_t(__cdecl *)(void))(uintptr_t)G32(0x1017d60c))();
            U32(entry, 0x10) = (uint32_t)stamp;
            U32(entry, 0x14) = (uint32_t)(stamp >> 32);
            *out = U32(entry, 0xc);
            return U32(entry, 0x18);
        }
    }
    return 0;
}
