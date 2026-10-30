#include "w7p.h"
#include "w2h.h"
#include "w4w.h"
#include "w6g.h"
#include "../util/access.h"
#include "../util/util.h"
#include "../core/w2a.h"
#include "../unsorted_1002/w2j.h"
#include "../unsorted_1002/w6d.h"
#include "../unsorted_1003/w4w.h"
#include "../unsorted_1003/w5c.h"
#include "../unsorted_1005/w3o.h"
#include "../unsorted_1005/w4v.h"
#include "../unsorted_1006/w2e.h"
#include "../unsorted_1006/w5y.h"
#include "../unsorted_100a/w6g.h"
#include "../unsorted_100d/w4v.h"
#include "../unsorted_100f/w2e.h"
#include "../unsorted_100f/w6h.h"
#include "../unsorted_100f/w6i.h"

// FUNCTION: HGSIM 0x100b39c3
int __cdecl Sim_100b39c3(int key, const char *name, uint32_t m0, uint32_t m1, uint32_t m2, uint32_t r0, uint32_t r1,
                         uint32_t r2, uint32_t r3, uint32_t r4, uint32_t r5, uint32_t value, int32_t parent,
                         uint32_t unused, uint32_t low, uint32_t high, uint32_t last)
{
    uint8_t block[0x30];
    uint8_t *entry;
    int index = Sim_100b38c5(key);

    (void)unused;
    if (index == -1)
        index = Sim_100b391c((uint32_t)key);
    if (index == -1)
        return index;
    entry = GBYTES(0x101a2100) + (uint32_t)index * 0x88;
    Sim_10060656((uint32_t)(uintptr_t)block, r0, r1, r2, r3, r4, r5);
    Sim_10060209(block, entry + 0x54);
    U32(entry, 0x30) = m0;
    U32(entry, 0x34) = m1;
    U32(entry, 0x38) = m2;
    U32(entry, 0x3c) = r0;
    U32(entry, 0x40) = r1;
    U32(entry, 0x44) = r2;
    U32(entry, 0x48) = r3;
    U32(entry, 0x4c) = r4;
    U32(entry, 0x50) = r5;
    Str_Copy((char *)entry, name);
    U32(entry, 0x14) = (uint32_t)parent;
    U32(entry, 0x10) = value;
    U32(entry, 0x28) = 0;
    if (parent == -2)
        U32(entry, 0x2c) = 0;
    else if (parent == -1)
        U32(entry, 0x2c) = (uint32_t)(uintptr_t)Sim_100a9830(0, 10);
    else
        U32(entry, 0x2c) = (uint32_t)(uintptr_t)Sim_100a9830(
            (void *)(uintptr_t)G32(0x101a212c + (uint32_t)parent * 0x88), 10);
    U32(entry, 0x18) |= low & 0x1ff;
    U32(entry, 0x18) |= (high << 12) & 0xf000;
    U32(entry, 0x1c) = last;
    W7P_CALL_100b469d();
    return index;
}

static void decode_name(char *out, const uint8_t *in)
{
    int i;

    for (i = 0; i < 10; i++)
        out[i] = (char)(uint8_t)(0x100 - (int32_t)(int8_t)in[i]);
}

// FUNCTION: HGSIM 0x100b8c30
int __cdecl Sim_100b8c30(int key, const char *name)
{
    int result = -1;
    char text[10];
    uint8_t *table = W7P_CALL_100282d4(G32(0x1014cf8c), key, G32(0x1014ced8), 1);
    uint8_t *rec;
    int i;

    if (table == 0)
        return result;
    rec = table + 0xc;
    decode_name(text, rec);
    for (i = 0; i < S16(table, 8) && Str_CompareNoCase(text, name) != 0; i++) {
        rec += 0xc;
        decode_name(text, rec);
    }
    if (Str_CompareNoCase(text, name) == 0)
        result = S16(rec, 0xa);
    Sim_1002828e(key, (const void *)(uintptr_t)G32(0x1014ced8));
    return result;
}

// FUNCTION: HGSIM 0x100b9675
void __cdecl Sim_100b9675(void *obj, void(__cdecl *fn)(uint32_t))
{
    if (obj == 0)
        W7P_CALL_100ff860(SIM_PTR(const char, 0x10150cdc), SIM_PTR(const char, 0x10150cb8), 0xd3);
    if (U32(obj, 0x18) != 0) {
        if (fn != 0)
            fn(U32(obj, 0x18));
        else if (G32(0x1017b914) != 0)
            G32(0x1017b8f8) = G32(0x1017b914);
        if (U32(obj, 0x18) != 0)
            Heap_FreeBlock(U32(obj, 0x18));
        G32(0x1017b8f8) = G32(0x1017b900);
    }
    if (obj != 0)
        Heap_FreeBlock((uint32_t)(uintptr_t)obj);
}

// FUNCTION: HGSIM 0x100bae0a
void *__cdecl Sim_100bae0a(const char *name, const char *ext)
{
    char path[0x104];
    uint8_t *rec;
    int i;

    if (G32(0x1017d600) == 0)
        return 0;
    rec = W7P_CALL_100ba1c9(G32(0x1017d600), ext);
    if (rec == 0)
        return 0;
    for (i = 0; i < S32(U32(rec, 8), 4); i++) {
        uint32_t dir = U32(U32(U32(rec, 8), 8), (uint32_t)i * 4);
        void *stream;

        W7P_CALL_100fc780(path, SIM_PTR(const char, 0x101511c0), dir, name, ext, (int)(int8_t)*GBYTES(0x1017d5f8));
        stream = Sim_100fd850(path, SIM_PTR(const char, 0x101511cc));
        if (stream != 0)
            return stream;
        W7P_CALL_100fc780(path, SIM_PTR(const char, 0x101511d0), dir, name, ext);
        stream = Sim_100fd850(path, SIM_PTR(const char, 0x101511dc));
        if (stream != 0)
            return stream;
    }
    return 0;
}

// FUNCTION: HGSIM 0x100bc451
void __cdecl Sim_100bc451(void)
{
    Sim_10053132();
    Sim_100db351();
    Sim_10027e6a();
    Sim_1003edda();
    Sim_100556c2();
    W7P_CALL_1005f949();
    Sim_100354ff();
    Sim_1002ab26();
}

// FUNCTION: HGSIM 0x100bd134
void __cdecl Sim_100bd134(void)
{
    if (G32(0x1017e1d0) != 0) {
        Sim_100bd69c((void *)(uintptr_t)G32(0x1017e1d0), W7P_FN_100bd1f3);
        G32(0x1017e1d0) = 0;
    }
    if (G32(0x1017e1c8) != 0) {
        Heap_FreeBlock(G32(0x1017e1c8));
        G32(0x1017e1c8) = 0;
    }
    if (G32(0x1017e1c0) != 0)
        W7P_CALL_100fd760(G32(0x1017e1c0));
    Sim_100bd1b6();
}

// FUNCTION: HGSIM 0x100bd1f3
void __cdecl Sim_100bd1f3(void *obj)
{
    if (U32(obj, 4) != 0) {
        Sim_100bd69c((void *)(uintptr_t)U32(obj, 4), 0);
        U32(obj, 4) = 0;
    }
    if (obj != 0)
        Heap_FreeBlock((uint32_t)(uintptr_t)obj);
}
