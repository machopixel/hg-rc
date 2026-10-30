#include "w8y.h"
#include "w2e.h"
#include "w3o.h"
#include "w3s.h"
#include "w7p.h"
#include "../util/access.h"
#include "../unsorted_100f/w2e.h"
#include "../unsorted_100f/w6h.h"
#include "../unsorted_100f/w6i.h"
#include "../unsorted_1010/w6i.h"
#include "../unsorted_1010/w7s.h"

typedef int(__cdecl *W8yUnpackFn)(void *out, uint32_t *size, uint32_t packed, uint32_t packedSize);
typedef void(__cdecl *W8yPackItemFn)(void *data, char *name, char *ext, uint32_t size);
typedef int(__cdecl *W8yElemCmp2Fn)(uint32_t, uint32_t);

#define ASSERT_AT(expr, file, line) W8Y_CALL_100ff860(SIM_PTR(const char, expr), SIM_PTR(const char, file), line)
/* The unpacker of a method: the function pointers 32 bytes apart from 0x1017dfd4. */
#define UNPACKER(method) ((W8yUnpackFn)(uintptr_t)G32(0x1017dfd4 + (uint32_t)(method) * 32u))
#define PACK_FILE G32(0x1017e1c0)

// FUNCTION: HGSIM 0x100bc610
int __cdecl Sim_100bc610(char *type)
{
    char name[0x10];
    char ext[4];
    uint32_t size;
    uint32_t packed;
    uint32_t i, j, pos;
    void *data;

    if (PACK_FILE == 0)
        return 0;
    Crt_StrUpper(type);
    for (i = 0; i < G32(0x1017e1c4); i++) {
        if (Str_CompareN((const char *)(uintptr_t)(G32(0x1017e1c8) + i * 0xc), type, 4) != 0)
            continue;
        pos = U32(G32(0x1017e1c8), i * 0xc + 4);
        for (j = 0; j < U32(G32(0x1017e1c8), i * 0xc + 8); j++) {
            data = Sim_100bc70d(pos, &size, &packed, name, ext);
            if (G32(0x1017e1cc) != 0)
                ((W8yPackItemFn)(uintptr_t)G32(0x1017e1cc))(data, name, ext, size);
            pos = pos + packed + 0x16;
        }
    }
    return 0;
}

// FUNCTION: HGSIM 0x100bc70d
void *__cdecl Sim_100bc70d(uint32_t pos, uint32_t *size, uint32_t *packed, char *name, char *ext)
{
    uint8_t header[0x16];
    uint32_t method;
    void *out;
    void *temp;
    int failed;

    W8Y_CALL_100feb20(PACK_FILE, pos, 0);
    W8Y_CALL_100fe830(header, 1, 0x16, PACK_FILE);
    if (name != 0)
        Str_CopyN(name, (const char *)header, 8);
    if (ext != 0) {
        Str_CopyN(ext, (const char *)header + 8, 4);
        ext[3] = 0;
    }
    method = U16(header, 0xc);
    *size = U32(header, 0xe);
    *packed = U32(header, 0x12);
    if (method == 0) {
        if (G32(0x1017b914) != 0)
            G32(0x1017b8f8) = G32(0x1017b914);
        out = Heap_Malloc(*packed);
        if (out == 0)
            ASSERT_AT(0x10151544, 0x10151524, 0x1c7);
        G32(0x1017b8f8) = G32(0x1017b900);
        G32(0x1017b914) = 0;
        W8Y_CALL_100fe830(out, 1, *packed, PACK_FILE);
        return out;
    }
    if (G32(0x1017b958) != 0 && (int32_t)*packed < GS32(0x1017b95c)) {
        W8Y_CALL_100fe830((void *)(uintptr_t)G32(0x1017b958), 1, *packed, PACK_FILE);
        if (G32(0x1017b914) != 0)
            G32(0x1017b8f8) = G32(0x1017b914);
        out = Heap_Malloc(*size);
        if (out == 0)
            ASSERT_AT(0x101514c4, 0x101514a4, 0x177);
        failed = UNPACKER(method)(out, size, G32(0x1017b958), *packed);
        if (failed != 0) {
            if (out != 0)
                Heap_FreeBlock((uint32_t)(uintptr_t)out);
            G32(0x1017b8f8) = G32(0x1017b900);
            G32(0x1017b914) = 0;
            return 0;
        }
        G32(0x1017b8f8) = G32(0x1017b900);
        G32(0x1017b914) = 0;
        return out;
    }
    temp = Heap_Malloc(*packed);
    if (temp == 0)
        ASSERT_AT(0x101514f0, 0x101514d0, 0x18d);
    W8Y_CALL_100fe830(temp, 1, *packed, PACK_FILE);
    if (G32(0x1017b914) != 0)
        G32(0x1017b8f8) = G32(0x1017b914);
    out = Heap_Malloc(*size);
    if (out == 0)
        ASSERT_AT(0x10151518, 0x101514f8, 0x195);
    failed = UNPACKER(method)(out, size, (uint32_t)(uintptr_t)temp, *packed);
    G32(0x1017b8f8) = G32(0x1017b900);
    if (temp != 0)
        Heap_FreeBlock((uint32_t)(uintptr_t)temp);
    if (failed != 0) {
        if (G32(0x1017b914) != 0)
            G32(0x1017b8f8) = G32(0x1017b914);
        if (out != 0)
            Heap_FreeBlock((uint32_t)(uintptr_t)out);
        G32(0x1017b8f8) = G32(0x1017b900);
        G32(0x1017b914) = 0;
        return 0;
    }
    return out;
}

// FUNCTION: HGSIM 0x100bca49
uint32_t __cdecl Sim_100bca49(const char *name, const char *ext, uint32_t *outSize)
{
    char key[8];
    uint32_t size;
    uint32_t unused;
    const char *list;
    uint32_t e;

    W8Y_CALL_100fc780(key, SIM_PTR(const char, 0x1015154c), ext, (int)(int8_t)*GBYTES(0x1017e1b8));
    list = Sim_100bcb52(key, (const void *)(uintptr_t)G32(0x1017e1d0));
    if (list == 0) {
        list = Sim_100bcb52(ext, (const void *)(uintptr_t)G32(0x1017e1d0));
        if (list == 0)
            return 0;
    }
    e = Sim_100bda9f((void *)(uintptr_t)U32(list, 4), (uint32_t)(uintptr_t)name,
                     SIM_FN(W8yElemCmp2Fn, 0x100bcba7));
    if (e == 0) {
        list = Sim_100bcb52(ext, (const void *)(uintptr_t)G32(0x1017e1d0));
        if (list == 0)
            return 0;
        e = Sim_100bda9f((void *)(uintptr_t)U32(list, 4), (uint32_t)(uintptr_t)name,
                         SIM_FN(W8yElemCmp2Fn, 0x100bcba7));
        if (e == 0)
            return 0;
    }
    if (outSize != 0)
        return (uint32_t)(uintptr_t)Sim_100bc70d(U32(e, 8), outSize, &unused, 0, 0);
    return (uint32_t)(uintptr_t)Sim_100bc70d(U32(e, 8), &size, &unused, 0, 0);
}

// FUNCTION: HGSIM 0x100bcd61
int __cdecl Sim_100bcd61(const char *name)
{
    char type[5];
    void *f;
    uint8_t *buf;
    uint8_t *p;
    uint32_t files, types, i;
    uint32_t rec;
    const char *list;
    int32_t len;

    f = Sim_100fd850(name, SIM_PTR(const char, 0x1015156c));
    if (f == 0)
        return 0;
    len = File_GetLength(U32(f, 0x10));
    buf = (uint8_t *)Heap_Malloc((uint32_t)len);
    if (buf == 0)
        ASSERT_AT(0x10151590, 0x10151570, 0xeb);
    W8Y_CALL_100fe830(buf, 1, (uint32_t)len, (uint32_t)(uintptr_t)f);
    p = buf;
    files = U32(p, 0);
    p += 4;
    types = U32(p, 0);
    p += 4;
    G32(0x1017e1d0) = (uint32_t)(uintptr_t)PtrArray_CreateSized((int)types);
    if (G32(0x1017e1d0) == 0)
        ASSERT_AT(0x101515b8, 0x10151598, 0xf8);
    for (i = 0; i < types; i++) {
        rec = (uint32_t)(uintptr_t)Heap_Malloc(8);
        if (rec == 0)
            ASSERT_AT(0x101515e4, 0x101515c4, 0xfe);
        Str_CopyN((char *)(uintptr_t)rec, (const char *)p, 4);
        p += 4;
        U32(rec, 4) = (uint32_t)(uintptr_t)PtrArray_CreateSized((int)U32(p, 0));
        p += 4;
        if (U32(rec, 4) == 0)
            ASSERT_AT(0x10151608, 0x101515e8, 0x103);
        PtrArray_Insert((void *)(uintptr_t)G32(0x1017e1d0), rec, 0);
    }
    G32(0x1017e1c4) = U32(p, 0);
    p += 4;
    if (G32(0x1017e1c4) != 0) {
        G32(0x1017e1c8) = (uint32_t)(uintptr_t)Heap_Malloc(G32(0x1017e1c4) * 0xc);
        if (G32(0x1017e1c8) == 0)
            ASSERT_AT(0x10151638, 0x10151618, 0x10e);
    } else {
        G32(0x1017e1c8) = 0;
    }
    for (i = 0; i < G32(0x1017e1c4); i++) {
        Str_CopyN((char *)(uintptr_t)(G32(0x1017e1c8) + i * 0xc), (const char *)p, 4);
        p += 4;
        U32(G32(0x1017e1c8), i * 0xc + 4) = U32(p, 0);
        p += 4;
        U32(G32(0x1017e1c8), i * 0xc + 8) = U32(p, 0);
        p += 4;
    }
    G32(0x1017e1bc) = U32(p, 0);
    p += 4;
    for (i = 1; i < G32(0x1017e1bc) + 1; i++) {
        Str_CopyN(SIM_PTR(char, 0x1017dfb8 + i * 32), (const char *)p, 0x10);
        p += 0x10;
    }
    for (i = 0; i < files; i++) {
        rec = (uint32_t)(uintptr_t)Heap_Malloc(0xc);
        if (rec == 0)
            ASSERT_AT(0x10151664, 0x10151644, 0x12d);
        Str_CopyN((char *)(uintptr_t)rec, (const char *)p, 8);
        p += 8;
        Str_CopyN(type, (const char *)p, 4);
        type[4] = 0;
        p += 4;
        U32(rec, 8) = U32(p, 0);
        p += 4;
        p += 4;
        list = Sim_100bcb52(type, (const void *)(uintptr_t)G32(0x1017e1d0));
        if (list == 0)
            ASSERT_AT(0x1015168c, 0x1015166c, 0x140);
        PtrArray_Insert((void *)(uintptr_t)U32(list, 4), rec, 0);
    }
    Heap_FreeBlock((uint32_t)(uintptr_t)buf);
    W8Y_CALL_100fd760(f);
    return 1;
}
