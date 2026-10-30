#include "w9e.h"
#include "w2f.h"
#include "w3o.h"
#include "w3s.h"
#include "w6g.h"
#include "w7p.h"
#include "../unsorted_100f/w2e.h"
#include "../unsorted_1010/w7s.h"
#include "../util/access.h"

#define STR(addr) SIM_PTR(const char, addr)
#define HEAP_KIND G32(0x1017b914)

static void W9e_Unlink(uint8_t *table, uint8_t *entry, int32_t bucket)
{
    if (U32(entry, 0x20) != 0)
        U32(U32(entry, 0x20), 0x1c) = U32(entry, 0x1c);
    else
        U32(U32(table, 0x10), (uint32_t)bucket * 4) = 0;
    if (U32(entry, 0x1c) != 0)
        U32(U32(entry, 0x1c), 0x20) = U32(entry, 0x20);
}

static int32_t W9e_Bucket(const uint8_t *table, const uint8_t *key)
{
    return (int32_t)(U32(key, 0) ^ U32(key, 4)) % S32(table, 0);
}

// FUNCTION: HGSIM 0x100b9203
int __cdecl Sim_100b9203(char *name, char *ext)
{
    uint8_t *table, *entry, *list;
    int32_t i, bucket;

    if (name != 0)
        Crt_StrUpper(name);
    if (ext != 0)
        Crt_StrUpper(ext);
    if (Str_Compare(ext, STR(0x10150c3c)) == 0 && G32(0x1017b904) != 0)
        HEAP_KIND = G32(0x1017b904);
    else if (Str_Compare(ext, STR(0x10150c40)) == 0)
        HEAP_KIND = G32(0x1017b908);
    else if (Str_Compare(ext, STR(0x10150c44)) == 0)
        HEAP_KIND = G32(0x1017b90c);
    else if (Str_Compare(ext, STR(0x10150c48)) == 0)
        HEAP_KIND = G32(0x1017b8fc);
    else if (G32(0x1017b910) != 0 &&
             (Str_Compare(ext, STR(0x10150c4c)) == 0 || Str_Compare(ext, STR(0x10150c50)) == 0 ||
              Str_Compare(ext, STR(0x10150c54)) == 0 || Str_Compare(ext, STR(0x10150c58)) == 0 ||
              Str_Compare(ext, STR(0x10150c5c)) == 0))
        HEAP_KIND = G32(0x1017b910);
    table = BYTES(Sim_100bda9f(BYTES(G32(0x1017d604)), (uint32_t)(uintptr_t)ext, W9E_CMP_100b971d));
    if (name == 0) {
        list = PtrArray_CreateSized(0x400);
        if (list == 0)
            W9E_ASSERT(STR(0x10150c84), STR(0x10150c60), 0x205);
        for (i = 0; i < S32(table, 0); i++) {
            for (entry = BYTES(U32(U32(table, 0x10), (uint32_t)i * 4)); entry != 0; entry = BYTES(U32(entry, 0x1c))) {
                if (S32(entry, 8) > 0)
                    continue;
                if (PtrArray_Insert(list, (uint32_t)(uintptr_t)entry, W9E_CMP_100b9748) == 0)
                    break;
            }
        }
        for (i = 0; i < S32(list, 4); i++) {
            entry = BYTES(U32(U32(list, 8), (uint32_t)i * 4));
            if (entry == 0)
                W9E_ASSERT(STR(0x10150cb4), STR(0x10150c90), 0x21a);
            S32(table, 0xc) = S32(table, 0xc) - S32(entry, 0xc);
            S32(table, 8) = S32(table, 8) - 1;
            W9e_Unlink(table, entry, U32(entry, 0x20) != 0 ? 0 : W9e_Bucket(table, entry));
            Sim_100b9675(entry, (void(__cdecl *)(uint32_t))(uintptr_t)U32(table, 0x20));
            if (S32(table, 0xc) < S32(table, 4))
                break;
        }
        Array_Clear(list);
        Sim_100bd69c(list, 0);
        HEAP_KIND = 0;
        return 1;
    }
    bucket = W9e_Bucket(table, (const uint8_t *)name);
    for (entry = BYTES(U32(U32(table, 0x10), (uint32_t)bucket * 4)); entry != 0; entry = BYTES(U32(entry, 0x1c))) {
        if (Str_CompareN((const char *)entry, name, 8) != 0 || S32(entry, 8) > 0)
            continue;
        W9e_Unlink(table, entry, bucket);
        S32(table, 8) = S32(table, 8) - 1;
        S32(table, 0xc) = S32(table, 0xc) - S32(entry, 0xc);
        Sim_100b9675(entry, (void(__cdecl *)(uint32_t))(uintptr_t)U32(table, 0x20));
        HEAP_KIND = 0;
        return 1;
    }
    HEAP_KIND = 0;
    return 0;
}
