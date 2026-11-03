#include "w8y.h"
#include "w2e.h"
#include "w2f.h"
#include "w3o.h"
#include "w3p.h"
#include "w3s.h"
#include "w5c.h"
#include "w6g.h"
#include "w7p.h"
#include "../util/access.h"
#include "../unsorted_100d/w2j.h"
#include "../unsorted_100f/w2e.h"
#include "../unsorted_100f/w2g.h"
#include "../unsorted_100f/w6h.h"
#include "../unsorted_100f/w6i.h"
#include "../unsorted_100f/w7q.h"
#include "../unsorted_1010/w6k.h"
#include "../unsorted_1010/w7s.h"
#include "../crt/hg_winheap.h"

typedef int(__cdecl *W8yDriverAskFn)(void);
typedef void(__cdecl *W8yDriverRectFn)(uint32_t, uint32_t, uint32_t, uint32_t);
typedef uint64_t(__cdecl *W8yTimeFn)(void);
typedef int(__cdecl *W8yElemCmpFn)(uint32_t, uint32_t);

#define CMP_FN(addr) SIM_FN(W8yElemCmpFn, addr)
#define ASSERT_AT(expr, file, line) W8Y_CALL_100ff860(SIM_PTR(const char, expr), SIM_PTR(const char, file), line)

// FUNCTION: HGSIM 0x100b8938
void __cdecl Sim_100b8938(void)
{
    int answer;

    /* the original first kills the loading animation's multimedia timer when [0x1017d5cc] holds one
       (timeKillEvent, then timeEndPeriod([0x10199448])); only the original Windows loading animation started it, and
       ours (w28_anim.c) never does: deleted (DECISIONS.md, "The last original code", step 2) */
    if (G32(0x1017d5d0) != 0) {
        Hg_HeapFree(G32(0x1017b8f8), 1, BYTES(G32(0x1017d5d0)));
        G32(0x1017d5d0) = 0;
    }
    if (G32(0x1017d5d4) != 0) {
        Hg_HeapFree(G32(0x1017b8f8), 1, BYTES(G32(0x1017d5d4)));
        G32(0x1017d5d4) = 0;
        G32(0x1017d5d8) = 0;
    }
    if (G32(0x1017b920) != 0)
        answer = ((W8yDriverAskFn)(uintptr_t)U32(G32(0x1015e688), 0x20))();
    else
        answer = -1;
    if (answer == 0) {
        G32(0x1017d5b8) = G32(0x109e3e20);
        G32(0x1017d5bc) = G32(0x109e3e24);
        W8Y_CALL_100eb070(SIM_PTR(void, 0x1017d5a0), 0xf0);
        ((W8yDriverRectFn)(uintptr_t)U32(G32(0x1015e68c), 0x1c))(G32(0x10199440), G32(0x10199444),
                                                                 G32(0x10199440) + 0x27f, G32(0x10199444) + 0x1df);
    }
}

/*
 * The cache heap 0x1017b914 for a resource type name: the first name takes 0x1017b904 (when that heap exists,
 * otherwise the next names are tried), the next three take 0x1017b908, 0x1017b90c, 0x1017b8fc; when 0x1017b910 is
 * set the last five names take it. names[] holds the addresses of the nine texts.
 */
static void pick_heap(const char *type, const uint32_t *names)
{
    int k;

    if (Str_Compare(type, SIM_PTR(const char, names[0])) == 0 && G32(0x1017b904) != 0) {
        G32(0x1017b914) = G32(0x1017b904);
        return;
    }
    if (Str_Compare(type, SIM_PTR(const char, names[1])) == 0) {
        G32(0x1017b914) = G32(0x1017b908);
        return;
    }
    if (Str_Compare(type, SIM_PTR(const char, names[2])) == 0) {
        G32(0x1017b914) = G32(0x1017b90c);
        return;
    }
    if (Str_Compare(type, SIM_PTR(const char, names[3])) == 0) {
        G32(0x1017b914) = G32(0x1017b8fc);
        return;
    }
    if (G32(0x1017b910) == 0)
        return;
    for (k = 4; k < 9; k++) {
        if (Str_Compare(type, SIM_PTR(const char, names[k])) == 0) {
            G32(0x1017b914) = G32(0x1017b910);
            return;
        }
    }
}

static const uint32_t kRemoveNames[9] = {0x10150ce0, 0x10150ce4, 0x10150ce8, 0x10150cec, 0x10150cf0,
                                         0x10150cf4, 0x10150cf8, 0x10150cfc, 0x10150d00};
static const uint32_t kLoadNames[9] = {0x101511e0, 0x101511e4, 0x101511e8, 0x101511ec, 0x101511f0,
                                       0x101511f4, 0x101511f8, 0x101511fc, 0x10151200};

static void drop_wtb_heap(void)
{
    if (G32(0x1017b914) == G32(0x1017b904)) {
        Hg_HeapDestroy(G32(0x1017b904));
        G32(0x1017b904) = 0;
    }
    G32(0x1017b914) = 0;
}

// FUNCTION: HGSIM 0x100b8ff9
void __cdecl Sim_100b8ff9(const char *name)
{
    void *f = Sim_100fd850(name, SIM_PTR(const char, 0x10150b1c));
    int32_t total, used, unused, i, j, k, n1, n2;
    uint32_t type, e;

    if (f == 0)
        return;
    Crt_FPrintf(f, SIM_PTR(const char, 0x10150b20));
    total = 0;
    used = 0;
    unused = 0;
    for (i = 0; i < S32(G32(0x1017d604), 4); i++) {
        type = U32(U32(G32(0x1017d604), 8), (uint32_t)i * 4);
        Crt_FPrintf(f, SIM_PTR(const char, 0x10150b3c), type + 0x18, U32(type, 0), U32(type, 8), U32(type, 0xc));
        n1 = 0;
        n2 = 0;
        total = (int32_t)((uint32_t)total + U32(type, 0xc));
        for (j = 0; j < S32(type, 0); j++) {
            k = 0;
            for (e = U32(U32(type, 0x10), (uint32_t)j * 4); e != 0; e = U32(e, 0x1c)) {
                if (U32(e, 8) != 0) {
                    n1++;
                    used = (int32_t)((uint32_t)used + U32(e, 0xc));
                } else {
                    n2++;
                    unused = (int32_t)((uint32_t)unused + U32(e, 0xc));
                }
                k++;
            }
            if (k >= 5)
                Crt_FPrintf(f, SIM_PTR(const char, 0x10150b68), j, k);
        }
        Crt_FPrintf(f, SIM_PTR(const char, 0x10150b88), n1);
        Crt_FPrintf(f, SIM_PTR(const char, 0x10150ba0), n2);
    }
    Crt_FPrintf(f, SIM_PTR(const char, 0x10150bb8), used);
    Crt_FPrintf(f, SIM_PTR(const char, 0x10150bd4), unused);
    Crt_FPrintf(f, SIM_PTR(const char, 0x10150bf0), total);
    Crt_FPrintf(f, SIM_PTR(const char, 0x10150c08));
    W8Y_CALL_100fd760(f);
    Sim_100d9fc0();
}

// FUNCTION: HGSIM 0x100b979f
int __cdecl Sim_100b979f(char *type)
{
    uint32_t t, e;
    uint8_t *arr;
    int32_t i;

    if (type != 0)
        Crt_StrUpper(type);
    t = Sim_100bda9f((void *)(uintptr_t)G32(0x1017d604), (uint32_t)(uintptr_t)type, CMP_FN(0x100b971d));
    pick_heap(type, kRemoveNames);
    if (t == 0) {
        drop_wtb_heap();
        return 0;
    }
    arr = (uint8_t *)PtrArray_CreateSized(0x400);
    if (arr == 0)
        ASSERT_AT(0x10150d28, 0x10150d04, 0x29b);
    for (i = 0; i < S32(t, 0); i++) {
        for (e = U32(U32(t, 0x10), (uint32_t)i * 4); e != 0; e = U32(e, 0x1c)) {
            if (S32(e, 8) > 0)
                continue;
            if (PtrArray_Insert(arr, e, 0) == 0)
                break;
        }
    }
    for (i = 0; i < S32(arr, 4); i++) {
        e = U32(U32(arr, 8), (uint32_t)i * 4);
        if (e == 0)
            ASSERT_AT(0x10150d58, 0x10150d34, 0x2b0);
        U32(t, 0xc) = U32(t, 0xc) - U32(e, 0xc);
        U32(t, 8) = U32(t, 8) - 1;
        if (U32(e, 0x20) != 0)
            U32(U32(e, 0x20), 0x1c) = U32(e, 0x1c);
        else
            U32(U32(t, 0x10), (uint32_t)((int32_t)(U32(e, 0) ^ U32(e, 4)) % S32(t, 0)) * 4) = 0;
        if (U32(e, 0x1c) != 0)
            U32(U32(e, 0x1c), 0x20) = U32(e, 0x20);
        Sim_100b9675((void *)(uintptr_t)e, (void(__cdecl *)(uint32_t))(uintptr_t)U32(t, 0x20));
    }
    Array_Clear(arr);
    Sim_100bd69c(arr, 0);
    drop_wtb_heap();
    return 1;
}

// FUNCTION: HGSIM 0x100b9b3f
uint32_t __cdecl Sim_100b9b3f(uint32_t data, const uint32_t *key, const char *type, uint32_t size)
{
    /* 1.2 (0x100a1f1f): the same code; only its out-of-memory asserts give source lines 2 higher */
    uint32_t t, e, head;
    uint64_t stamp;
    int32_t slot;

    t = Sim_100bda9f((void *)(uintptr_t)G32(0x1017d604), (uint32_t)(uintptr_t)type, CMP_FN(0x100b971d));
    if (t == 0) {
        Sim_100d9fc0();
        t = (uint32_t)(uintptr_t)Heap_Malloc(0x24);
        if (t == 0)
            ASSERT_AT(0x10150dc0, 0x10150d9c, 0x2f3);
        Str_CopyN((char *)(uintptr_t)(t + 0x18), type, 8);
        U32(t, 0x20) = 0;
        U32(t, 0) = 0x400;
        U32(t, 4) = 0x100000;
        U32(t, 8) = 0;
        U32(t, 0xc) = 0;
        U32(t, 0x10) = (uint32_t)(uintptr_t)Heap_Malloc(U32(t, 0) << 2);
        if (U32(t, 0x10) == 0)
            ASSERT_AT(0x10150dec, 0x10150dc8, 0x2fd);
        Mem_Fill((void *)(uintptr_t)U32(t, 0x10), 0, U32(t, 0) << 2);
        U32(t, 0x14) = (uint32_t)(uintptr_t)PtrArray_Create();
        if (U32(t, 0x14) == 0)
            ASSERT_AT(0x10150e1c, 0x10150df8, 0x301);
        if (Sim_100bdc5d((SortedList100b *)(uintptr_t)G32(0x1017d604), (int32_t)t,
                         SIM_FN(int(__cdecl *)(int32_t, int32_t), 0x100b9db0)) == 0)
            ASSERT_AT(0x10150e50, 0x10150e2c, 0x305);
    }
    if (size + U32(t, 0xc) >= U32(t, 4))
        W8Y_CALL_100b9203(0, (const char *)(uintptr_t)(t + 0x18));
    U32(t, 8) = U32(t, 8) + 1;
    U32(t, 0xc) = U32(t, 0xc) + size;
    slot = (int32_t)(key[0] ^ key[1]) % S32(t, 0);
    e = (uint32_t)(uintptr_t)Heap_Malloc(0x24);
    if (e == 0)
        ASSERT_AT(0x10150e7c, 0x10150e58, 0x31f);
    head = U32(U32(t, 0x10), (uint32_t)slot * 4);
    U32(e, 8) = 1;
    U32(e, 0x20) = 0;
    if (head == 0) {
        U32(e, 0x1c) = 0;
    } else {
        U32(e, 0x1c) = head;
        U32(head, 0x20) = e;
    }
    Str_CopyN((char *)(uintptr_t)e, (const char *)key, 8);
    U32(U32(t, 0x10), (uint32_t)slot * 4) = e;
    U32(e, 0xc) = size;
    U32(e, 0x18) = data;
    stamp = ((W8yTimeFn)(uintptr_t)G32(0x1017d60c))();
    U32(e, 0x10) = (uint32_t)stamp;
    U32(e, 0x14) = (uint32_t)(stamp >> 32);
    return (uint32_t)stamp;
}

// FUNCTION: HGSIM 0x100b9eab
void __cdecl Sim_100b9eab(char *packName, const char *cacheFile)
{
    Mem_Fill(SIM_PTR(void, 0x101991a0), 0, 0x10);
    if (W8Y_CALL_100b8f30() != 0) {
        Mem_Move(SIM_PTR(void, 0x101991a0), SIM_PTR(const void, 0x10199194), 4);
        Mem_Move(SIM_PTR(void, 0x101991a4), SIM_PTR(const void, 0x1019917c), 4);
        Mem_Move(SIM_PTR(void, 0x101991a8), SIM_PTR(const void, 0x10199190), 4);
        G32(0x1017d608) =
            Str_CompareNoCase(SIM_PTR(const char, 0x10150ea0), SIM_PTR(const char, 0x101991a0)) == 0 ? 1 : 0;
    } else {
        G32(0x1017d608) = 0;
    }
    if (G32(0x1017d608) != 0)
        G32(0x1017d60c) = (uint32_t)(uintptr_t)SIM_PTR(void, 0x100034d1);
    else
        G32(0x1017d60c) = (uint32_t)(uintptr_t)SIM_PTR(void, 0x10002b62);
    if (packName != 0)
        G32(0x1017d5fc) =
            (uint32_t)Sim_100bcbcf(packName, (uint32_t)(uintptr_t)SIM_PTR(void, 0x100023c4), (int)W8Y_CALL_100b9dde());
    else
        G32(0x1017d5fc) = 0;
    if (G32(0x1017d5fc) == 0)
        Sim_100b9fa5();
    Sim_100ba304(cacheFile);
}

// FUNCTION: HGSIM 0x100b9fa5
void __cdecl Sim_100b9fa5(void)
{
    char line[0x100];
    void *f;
    char *tok;
    uint32_t rec;

    G32(0x1017d600) = (uint32_t)(uintptr_t)PtrArray_CreateSized(0x40);
    if (G32(0x1017d600) == 0)
        ASSERT_AT(0x10150ed4, 0x10150eb0, 0x138);
    f = Sim_100fd850(SIM_PTR(const char, 0x10150ee4), SIM_PTR(const char, 0x10150ee0));
    if (f == 0)
        return;
    Mem_Fill(line, 0, 0x100);
    while (Stream_ReadLine(line, 0x100, (uint8_t *)f) != 0) {
        if (line[0] == '#')
            continue;
        W8Y_CALL_100ba285(line);
        tok = Str_NextToken_100fd560(line, SIM_PTR(const char, 0x10150eec));
        W8Y_CALL_100ba21c(tok);
        rec = W8Y_CALL_100ba1c9(G32(0x1017d600), tok);
        if (rec == 0) {
            rec = (uint32_t)(uintptr_t)Heap_Malloc(0xc);
            if (rec == 0)
                ASSERT_AT(0x10150f14, 0x10150ef0, 0x151);
            Str_CopyN((char *)(uintptr_t)rec, tok, 8);
            U32(rec, 8) = (uint32_t)(uintptr_t)PtrArray_Create();
            if (U32(rec, 8) == 0)
                ASSERT_AT(0x10150f40, 0x10150f1c, 0x155);
            if (PtrArray_Insert((void *)(uintptr_t)G32(0x1017d600), rec, 0) == 0)
                ASSERT_AT(0x10150f70, 0x10150f4c, 0x158);
        }
        for (tok = Str_NextToken_100fd560(0, SIM_PTR(const char, 0x10150f78)); tok != 0;
             tok = Str_NextToken_100fd560(0, SIM_PTR(const char, 0x10150fa8))) {
            char *dir = Sim_1010c9f0(W8Y_CALL_100ba252(tok));

            if (PtrArray_Insert((void *)(uintptr_t)U32(rec, 8), (uint32_t)(uintptr_t)dir, 0) == 0)
                ASSERT_AT(0x10150fa0, 0x10150f7c, 0x161);
        }
    }
    W8Y_CALL_100fd760(f);
}

// FUNCTION: HGSIM 0x100ba304
void __cdecl Sim_100ba304(const char *name)
{
    /* 1.2 (0x100a26e4): a guard for a line with hash size 0 (fixed, below); the asserts give 1.2's source lines */
    char line[0x100];
    void *f;
    char *tok;
    uint32_t t, old;
    int fresh;

    G32(0x1017d604) = (uint32_t)(uintptr_t)PtrArray_CreateSized(0x20);
    if (G32(0x1017d604) == 0)
        ASSERT_AT(0x10150fd0, 0x10150fac, 0x367);
    f = Sim_100fd850(name, SIM_PTR(const char, 0x10150fdc));
    if (f == 0)
        return;
    while (Stream_ReadLine(line, 0x100, (uint8_t *)f) != 0) {
        if (line[0] == '#')
            continue;
        tok = Str_NextToken_100fd560(line, SIM_PTR(const char, 0x10150fe0));
        W8Y_CALL_100ba21c(tok);
        t = Sim_100bda9f((void *)(uintptr_t)G32(0x1017d604), (uint32_t)(uintptr_t)tok, CMP_FN(0x100b971d));
        fresh = t == 0;
        if (t == 0) {
            t = (uint32_t)(uintptr_t)Heap_Malloc(0x24);
            if (t == 0)
                ASSERT_AT(0x10151008, 0x10150fe4, 0x37c);
            U32(t, 0x20) = 0;
            Str_CopyN((char *)(uintptr_t)(t + 0x18), Crt_StrUpper(tok), 8);
        }
        tok = Str_NextToken_100fd560(0, SIM_PTR(const char, 0x10151010));
        if (tok == 0) {
            if (t != 0)
                Heap_FreeBlock(t);
            continue;
        }
        old = fresh ? 0 : U32(t, 0);
        U32(t, 0) = (uint32_t)Crt_AToI(tok);
        if (S32(t, 0) == -1)
            U32(t, 0) = 0x400;
        if (U32(t, 0) == 0) {
            /* Fix (DECISIONS.md, "Cache size 0"): 1.2 keeps the type with no slot table (+0x10 = 0), but filing or
             * finding a file of it divides by the size 0 (1.0's own CACHE.INI has "tex 0 0"). The line is left out
             * instead: the type is made with the defaults when its first file is filed (Sim_100b9b3f), as for a
             * type the file does not list */
            if (fresh)
                Heap_FreeBlock(t);
            else
                U32(t, 0) = old;
            continue;
        }
        tok = Str_NextToken_100fd560(0, SIM_PTR(const char, 0x10151014));
        if (tok == 0) {
            if (t != 0)
                Heap_FreeBlock(t);
            continue;
        }
        U32(t, 4) = (uint32_t)Crt_AToI(tok);
        if (S32(t, 4) == -1)
            U32(t, 4) = 0x100000;
        U32(t, 0x10) = (uint32_t)(uintptr_t)Heap_Malloc(U32(t, 0) << 2);
        if (U32(t, 0x10) == 0)
            ASSERT_AT(0x1015103c, 0x10151018, 0x39a);
        Mem_Fill((void *)(uintptr_t)U32(t, 0x10), 0, U32(t, 0) << 2);
        U32(t, 0x14) = (uint32_t)(uintptr_t)PtrArray_Create();
        if (U32(t, 0x14) == 0)
#ifdef W8Y_MUTATE12 /* the checker's negative check only */
            ASSERT_AT(0x1015106c, 0x10151048, 0x39a);
#else
            ASSERT_AT(0x1015106c, 0x10151048, 0x3a3);
#endif
        U32(t, 8) = 0;
        U32(t, 0xc) = 0;
        if (PtrArray_Insert((void *)(uintptr_t)G32(0x1017d604), t, SIM_FN(int(__cdecl *)(uint32_t, uint32_t), 0x100b9db0))
            == 0)
            ASSERT_AT(0x101510a0, 0x1015107c, 0x3aa);
    }
    W8Y_CALL_100fd760(f);
}

// FUNCTION: HGSIM 0x100baff4
uint32_t __cdecl Sim_100baff4(char *name, char *ext, uint32_t *outSize)
{
    char small[8];
    char path[0x100];
    uint32_t size;
    uint32_t r;

    Crt_StrUpper(name);
    Crt_StrUpper(ext);
    pick_heap(ext, kLoadNames);
    r = Sim_100baf3f((uint32_t *)name, (uint32_t)(uintptr_t)ext, &size);
    if (r != 0) {
        if (outSize != 0)
            *outSize = size;
        G32(0x1017b914) = 0;
        return r;
    }
    *GBYTES(0x1017d5f8) = (uint8_t)W8Y_CALL_100b9dde();
    W8Y_CALL_100fc780(small, SIM_PTR(const char, 0x10151204), ext, (int)(int8_t)*GBYTES(0x1017d5f8));
    W8Y_CALL_100fc780(path, SIM_PTR(const char, 0x1015120c), name, small);
    W8Y_CALL_100fc780(path, SIM_PTR(const char, 0x10151214), name, ext);
    if (G32(0x1017d5fc) != 0) {
        r = Sim_100bca49(name, ext, &size);
        if (r == 0) {
            G32(0x1017b914) = 0;
            return r;
        }
        Sim_100b9b3f(r, (const uint32_t *)name, ext, size);
        if (outSize != 0)
            *outSize = size;
        G32(0x1017b914) = 0;
        return r;
    }
    G32(0x1017b914) = 0;
    return 0;
}

static const uint32_t kLoadNames7d5[9] = {0x101510f8, 0x101510fc, 0x10151100, 0x10151104, 0x10151108,
                                          0x1015110c, 0x10151110, 0x10151114, 0x10151118};

/* A whole file into a block of the chosen heap, closed and filed under (name, ext, size): the three file paths of
   Sim_100ba7d5 (their asserts differ only in the line and texts). */
static void *W25_ReadFile(uint32_t f, char *name, char *ext, uint32_t *outSize, uint32_t expr, uint32_t file,
                          int line)
{
    uint32_t size;
    void *block;

    W8Y_CALL_100feb20(f, 0, 2);
    size = (uint32_t)W8Y_CALL_100fe970(f);
    W8Y_CALL_100feb20(f, 0, 0);
    if (G32(0x1017b914) != 0)
        G32(0x1017b8f8) = G32(0x1017b914);
    block = W8Y_CALL_100fef30(size);
    if (block == 0)
        ASSERT_AT(expr, file, line);
    G32(0x1017b8f8) = G32(0x1017b900);
    W8Y_CALL_100fe830(block, 1, size, f);
    W8Y_CALL_100fd760((void *)(uintptr_t)f);
    Sim_100b9b3f((uint32_t)(uintptr_t)block, (const uint32_t *)name, ext, size);
    if (outSize != 0)
        *outSize = size;
    G32(0x1017b914) = 0;
    return block;
}

/* round 28: called at the start of every file load when set (the loading animation's tick, W28_AnimPoll) */
void(__cdecl *g_w8yLoadPoll)(void);
/* round 31: 1.2's Patch folder first (only with the Vulkan renderer; the hook layer sets it at sim load) */
int g_w8yPatch;

// FUNCTION: HGSIM 0x100ba7d5
void *__cdecl Sim_100ba7d5(char *name, char *ext, uint32_t *outSize)
{
    char small[8];
    char path[0x100];
    uint32_t size;
    uint32_t r, f;

    if (g_w8yLoadPoll != 0)
        g_w8yLoadPoll();
    Crt_StrUpper(name);
    Crt_StrUpper(ext);
    pick_heap(ext, kLoadNames7d5);
    r = Sim_100baf3f((uint32_t *)name, (uint32_t)(uintptr_t)ext, &size);
    if (r != 0) {
        if (outSize != 0)
            *outSize = size;
        G32(0x1017b914) = 0;
        return (void *)(uintptr_t)r;
    }
    *GBYTES(0x1017d5f8) = (uint8_t)W8Y_CALL_100b9dde();
    W8Y_CALL_100fc780(small, SIM_PTR(const char, 0x1015111c), ext, (int)(int8_t)*GBYTES(0x1017d5f8));
    if (g_w8yPatch) {
        /* 1.2 (0x100a2bc9): "Patch\name.ext<letter>", then "Patch\name.ext", before the 1.0 files */
        W8Y_CALL_100fc780(path, "%s\\%s.%s", "Patch", name, small);
        f = (uint32_t)(uintptr_t)Sim_100fd850(path, SIM_PTR(const char, 0x1015112c));
#ifdef W31B_MUTATE /* the checker's negative check only: the language file is passed over */
        f = 0;
#endif
        if (f != 0)
            return W25_ReadFile(f, name, ext, outSize, 0x10151154, 0x10151130, 0x44f);
        W8Y_CALL_100fc780(path, "%s\\%s.%s", "Patch", name, ext);
        f = (uint32_t)(uintptr_t)Sim_100fd850(path, SIM_PTR(const char, 0x10151164));
        if (f != 0)
            return W25_ReadFile(f, name, ext, outSize, 0x1015118c, 0x10151168, 0x479);
    }
    W8Y_CALL_100fc780(path, SIM_PTR(const char, 0x10151124), name, small);
    f = (uint32_t)(uintptr_t)Sim_100fd850(path, SIM_PTR(const char, 0x1015112c));
    if (f != 0)
        return W25_ReadFile(f, name, ext, outSize, 0x10151154, 0x10151130, 0x442);
    W8Y_CALL_100fc780(path, SIM_PTR(const char, 0x1015115c), name, ext);
    f = (uint32_t)(uintptr_t)Sim_100fd850(path, SIM_PTR(const char, 0x10151164));
    if (f != 0)
        return W25_ReadFile(f, name, ext, outSize, 0x1015118c, 0x10151168, 0x46c);
    if (G32(0x1017d5fc) != 0) {
        r = Sim_100bca49(name, ext, &size);
        if (r == 0) {
            G32(0x1017b914) = 0;
            return 0;
        }
#ifdef W25C_MUTATE /* the checker's negative check only */
        size++;
#endif
        Sim_100b9b3f(r, (const uint32_t *)name, ext, size);
        if (outSize != 0)
            *outSize = size;
        G32(0x1017b914) = 0;
        return (void *)(uintptr_t)r;
    }
    f = (uint32_t)(uintptr_t)Sim_100bae0a(name, ext);
    if (f != 0)
        return W25_ReadFile(f, name, ext, outSize, 0x101511b8, 0x10151194, 0x4bb);
    G32(0x1017b914) = 0;
    return 0;
}
