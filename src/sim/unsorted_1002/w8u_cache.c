#include "w8u.h"
#include "w2f.h"
#include "w2j.h"
#include "w7l.h"
#include "../unsorted_1007/w2e.h"
#include "../unsorted_100a/w5b.h"
#include "../unsorted_100d/w2j.h"
#include "../unsorted_100f/w6h.h"
#include "../unsorted_100f/w7q.h"
#include "../util/access.h"

/* One line of the cache dump: id (word +0), type (dword +4 as 4 characters), lock (word +2), HeapSize. */
static void W8u_DumpNode(void *f, uint32_t format, uint8_t *node, char *type)
{
    uint32_t size;

    *(u32a *)type = U32(node, 4);
    size = SIM_API(W8uHeapSizeFn, 0x10ac5638)(G32(0x1017b8f8), 1, node);
    Crt_FPrintf(f, SIM_PTR(const char, format), (uint32_t)U16(node, 0), type, (int)S16(node, 2), size);
}

// FUNCTION: HGSIM 0x1002812f
void __cdecl Cache_DumpLog(void)
{
    char name[0x64];
    char type[5];
    uint32_t n = G32(0x1015dc34);
    void *f;
    uint8_t *node;
    int i;

    G32(0x1015dc34) = n + 1;
    W8U_SPRINTF_1002(name, SIM_PTR(const char, 0x10135068), n);                 /* "dbugcch%d.log" */
    f = Sim_100fd850(name, SIM_PTR(const char, 0x10135078));                     /* "w" */
    type[4] = 0;
    Crt_FPrintf(f, SIM_PTR(const char, 0x1013507c));                            /* "Cache table\n..." */
    for (i = 0; i < 0x3f1; i++)
        for (node = BYTES(U32(G32(0x1015dc30), 4 * i)); node != 0; node = BYTES(U32(node, 8)))
            W8u_DumpNode(f, 0x101350a4, node, type);
    Crt_FPrintf(f, SIM_PTR(const char, 0x101350cc));                            /* "\nPurge list\n..." */
    for (node = BYTES(G32(0x109f13f0)); node != 0; node = BYTES(U32(node, 0xc)))
        W8u_DumpNode(f, 0x101350f4, node, type);
    W8U_FCLOSE_1002(f);
}

/* The two "memory low" / "cache full" messages: Sim_1007df6c(text, [0x109b98c0], 0x712, 100). */
static void W8u_CacheMessage(uint32_t text)
{
    Sim_1007df6c(SIM_PTR(const char, text), (const char *)(uintptr_t)G32(0x109b98c0), 0x712, 100);
}

// FUNCTION: HGSIM 0x100282d4
void *__cdecl Cache_UseItem(uint32_t a, int key, const void *name)
{
    uint8_t *node;
    uint8_t *block;
    int size;
    int32_t slot;
    const int8_t *n = (const int8_t *)name;

    Sim_100d9fc0();     /* the original passes ("Calling UseItem, please convert...", 0x1b7, "...CACHE.C") */
    if (key < 0)
        return 0;
    node = Sim_10027f33(key, name);
    if (node != 0) {
        Sim_10027cce(node);
        return node + 0x14;
    }
    if (GS32(0x109f13e0) >= 1000) {
        if (G32(0x109f13f0) != 0) {
            Sim_10027fb8((void *)(uintptr_t)G32(0x109f13f0));
        } else {
            W8u_CacheMessage(G32(0x1015f1e8) != 0 ? 0x101351e0 : 0x10135204);
            return 0;
        }
    }
    size = Sim_100a8068((int)a, (int)(uintptr_t)name, key);
    if (size <= 0)
        return 0;
    for (;;) {
        block = SIM_API(W8uHeapAllocFn, 0x10ac5658)(G32(0x1017b8f8), 1, (uint32_t)size + 0x14);
        if (block != 0)
            break;
        if (G32(0x109f13f0) == 0) {
            W8u_CacheMessage(0x10135238);
            return 0;
        }
        Sim_10027fb8((void *)(uintptr_t)G32(0x109f13f0));
    }
    if (W8U_CALL_100a836e(a, (void *)name, (uint32_t)key & 0xffff, block + 0x14) == -1)
        return 0;
    slot = (int32_t)((uint32_t)key + (uint32_t)n[0] + (uint32_t)n[1] + (uint32_t)n[2] + (uint32_t)n[3]) % 0x3f1;
    U32(block, 8) = U32(G32(0x1015dc30), slot * 4);
    U32(G32(0x1015dc30), slot * 4) = (uint32_t)(uintptr_t)block;
    U16(block, 2) = 1;
    U16(block, 0) = (uint16_t)key;
    U32(block, 4) = U32(name, 0);
    U32(block, 0xc) = 0;
    U32(block, 0x10) = 0;
    G32(0x109f13e0) = G32(0x109f13e0) + 1;
    return block + 0x14;
}
