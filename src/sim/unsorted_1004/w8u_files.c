#include "w8u.h"
#include "w5c.h"
#include "../unsorted_1002/w2j.h"
#include "../unsorted_1001/w2i.h"
#include "../unsorted_1002/w8u.h"
#include "../unsorted_100f/w2e.h"
#include "../unsorted_100f/w6h.h"
#include "../unsorted_100f/w7q.h"
#include "../unsorted_1010/w6i.h"
#include "../unsorted_1010/w6k.h"
#include "../win/w4v.h"
#include "../util/w3o.h"

// FUNCTION: HGSIM 0x100461e8
void *__cdecl File_LoadWhole(const char *name, void *buffer)
{
    void *f = Sim_100fd850(name, SIM_PTR(const char, 0x1013b0dc));     /* "r" */
    int32_t size;
    void *p;

    if (f == 0)
        return 0;
    W8U_FSEEK_1004(f, 0, 2);
    size = Crt_FTell(f);
    W8U_FSEEK_1004(f, 0, 0);
    if (buffer == 0)
        p = SIM_API(W8u4HeapAllocFn, 0x10ac5658)(G32(0x1017b8f8), 1, (uint32_t)size);
    else
        p = buffer;
    W8U_FREAD_1004(p, 1, (uint32_t)size, f);
    W8U_FCLOSE_1004(f);
    return p;
}

// FUNCTION: HGSIM 0x100463bf
int __cdecl Sim_100463bf(const void *name)
{
    uint32_t ids[32];       /* [ebp-0x80]: the original has room for 32 */
    uint32_t loaded;        /* [ebp-0x84], written by 0x100b8d6c */
    uint8_t *data;
    uint32_t count, nvals, off;
    uint32_t base;
    int32_t i, k;

    data = W8U_CALL_100b8d6c(name, G32(0x1014cee4), G32(0x1014cf50), 3, &loaded,
                             SIM_PTR(const char, 0x10134f98));        /* "ANFLOBJICID" */
    if (data == 0)
        return 0;
    count = U32(data, 0);
    nvals = U32(data, 4);
    off = 8;
    base = Sim_10019d8c();
    for (i = 0; i < (int32_t)count; i++) {
        int32_t id = S32(data, off);
        uint32_t kind;
        uint8_t *vals;
        uint8_t *rec;

        off += 4;
        kind = U32(data, off);
        off += 4;
        if (id >= 0x20)
            return 0;
        id = (int32_t)((uint32_t)id + base);
        if (id >= 0x320)
            return 0;
        vals = data + off;
        off += nvals * 4;
        G32(0x10a1d6c0 + 4 * (uint32_t)id) = (uint32_t)(uintptr_t)Heap_AllocRaw(0x14);
        if (G32(0x10a1d6c0 + 4 * (uint32_t)id) == 0)
            return 0;
        if (nvals != 0)
            for (k = 0; k < (int32_t)nvals; k++) {
                if ((int32_t)kind < 3)
                    FL(vals, 4 * k) = (float)S32(vals, 4 * k);
                else
                    FL(vals, 4 * k) = (float)((double)S32(vals, 4 * k) * (double)GFL(0x101115d4));
            }
        rec = BYTES(G32(0x10a1d6c0 + 4 * (uint32_t)id));
        U32(rec, 0xc) = (uint32_t)(uintptr_t)vals;
        rec = BYTES(G32(0x10a1d6c0 + 4 * (uint32_t)id));
        U32(rec, 8) = kind;
        rec = BYTES(G32(0x10a1d6c0 + 4 * (uint32_t)id));
        U32(rec, 4) = nvals;
        rec = BYTES(G32(0x10a1d6c0 + 4 * (uint32_t)id));
        U32(rec, 0) = S16(name, 0) == -1;
        ids[i] = (uint32_t)id;
    }
    for (i = 0; i < (int32_t)count; i++)
        U32(G32(0x10a1d6c0 + 4 * ids[i]), 0x10) = (uint32_t)(uintptr_t)(data + off);
    return 1;
}

// FUNCTION: HGSIM 0x100469f2
int __cdecl Sim_100469f2(const char *name, const void *data)
{
    uint32_t h = Sim_10109980((uint32_t)(uintptr_t)name, 0x8101, 0x80);
    void *item;

    if (h == 0xffffffffu)
        return 0;
    item = Cache_UseItem(G32(0x1014cf8c), 0xd, (const void *)(uintptr_t)G32(0x1014ced8));   /* the original adds a 4th argument, 1 */
    if (item == 0) {
        W8U_CLOSE_1004(h);
        return 0;
    }
    W8U_WRITE_1004(h, item, 0x20);
    W8U_WRITE_1004(h, data, 0x300);
    W8U_WRITE_1004(h, (const void *)(uintptr_t)G32(0x109e3e20), G32(0x109e3de4));
    W8U_CLOSE_1004(h);
    Sim_1002828e(0xd, (const void *)(uintptr_t)G32(0x1014ced8));
    return 1;
}

// FUNCTION: HGSIM 0x10046ac4
int32_t __cdecl File_ReadAll(const char *name, uint32_t *size, uint32_t *buffer, const uint32_t *alloc)
{
    /* The original passes no third argument: _open only reads it with O_CREAT (0x100), not set here. */
    int32_t h = (int32_t)Sim_10109980((uint32_t)(uintptr_t)name, 0x8000, 0);

    if (h == -1)
        return -1;
    *size = (uint32_t)File_GetLength((uint32_t)h);
    if (alloc == 0)
        *buffer = (uint32_t)(uintptr_t)SIM_API(W8u4HeapAllocFn, 0x10ac5658)(G32(0x1017b8f8), 1, *size);
    else {
        /* the original also reads *alloc and passes it as a second argument, which Heap_AllocRaw does not use */
        volatile uint32_t unused = *alloc;

        (void)unused;
        *buffer = (uint32_t)(uintptr_t)Heap_AllocRaw(*size);
    }
    if (*buffer == 0) {
        W8U_CLOSE_1004((uint32_t)h);
        return -1;
    }
    if ((uint32_t)W8U_READ_1004((uint32_t)h, (void *)(uintptr_t)*buffer, *size) != *size) {
        if (alloc == 0)
            SIM_API(W8u4HeapFreeFn, 0x10ac5634)(G32(0x1017b8f8), 1, (void *)(uintptr_t)*buffer);
        W8U_CLOSE_1004((uint32_t)h);
        return -1;
    }
    return h;
}

// FUNCTION: HGSIM 0x10046ba1
int32_t __cdecl Sim_10046ba1(const char *name, uint32_t *out)
{
    uint32_t size, buffer;      /* [ebp-4], [ebp-0xc] */
    int32_t h;

    *out = 0;
    h = File_ReadAll(Sim_100474db(name), &size, &buffer, 0);
    if (h != -1) {
        W8U_CLOSE_1004((uint32_t)h);
        *out = buffer;
    }
    return h;
}

// FUNCTION: HGSIM 0x10046bf3
int __cdecl Stats_WriteKills(const char *name, const uint8_t *rec)
{
    void *f = Sim_100fd850(name, SIM_PTR(const char, 0x1013b0e0));     /* "wt" */
    int i, j;

    if (f == 0)
        return -1;
    Crt_FPrintf(f, SIM_PTR(const char, 0x1013b0e4));                    /* "USER\n" */
    Crt_FPrintf(f, SIM_PTR(const char, 0x1013b0ec), (uint32_t)U16(rec, 7));
    Crt_FPrintf(f, SIM_PTR(const char, 0x1013b104), (uint32_t)U16(rec, 9));
    Crt_FPrintf(f, SIM_PTR(const char, 0x1013b11c), (uint32_t)U16(rec, 0xb));
    for (i = 0; i < 0xb; i++)
        Crt_FPrintf(f, SIM_PTR(const char, 0x1013b134), i, (uint32_t)U16(rec, 0x46 + 2 * i));
    for (i = 0; i < 0xb; i++)
        Crt_FPrintf(f, SIM_PTR(const char, 0x1013b150), i, (uint32_t)U16(rec, 0x5c + 2 * i));
    for (i = 0; i < 0xb; i++)
        Crt_FPrintf(f, SIM_PTR(const char, 0x1013b170), i, (uint32_t)U16(rec, 0x72 + 2 * i));
    for (i = 0; i < 7; i++)
        Crt_FPrintf(f, SIM_PTR(const char, 0x1013b190), i, (uint32_t)U16(rec, 0x116 + 2 * i));
    for (i = 0; i < 7; i++)
        Crt_FPrintf(f, SIM_PTR(const char, 0x1013b1b8), i, (uint32_t)U16(rec, 0x124 + 2 * i));
    Crt_FPrintf(f, SIM_PTR(const char, 0x1013b1e0));                    /* "STARMATES\n" */
    Crt_FPrintf(f, SIM_PTR(const char, 0x1013b1ec), (uint32_t)U16(rec, 0x20));
    Crt_FPrintf(f, SIM_PTR(const char, 0x1013b208), (uint32_t)U16(rec, 0x22));
    Crt_FPrintf(f, SIM_PTR(const char, 0x1013b224), (uint32_t)U16(rec, 0x24));
    for (i = 0; i < 2; i++)
        for (j = 0; j < 0xb; j++)
            Crt_FPrintf(f, SIM_PTR(const char, 0x1013b240), i, j, (uint32_t)U16(rec, 0x88 + 0x16 * i + 2 * j));
    for (i = 0; i < 2; i++)
        for (j = 0; j < 0xb; j++)
            Crt_FPrintf(f, SIM_PTR(const char, 0x1013b264), i, j, (uint32_t)U16(rec, 0xb4 + 0x16 * i + 2 * j));
    for (i = 0; i < 2; i++)
        for (j = 0; j < 0xb; j++)
            Crt_FPrintf(f, SIM_PTR(const char, 0x1013b28c), i, j, (uint32_t)U16(rec, 0xe0 + 0x16 * i + 2 * j));
    W8U_FCLOSE_1004(f);
    return 0;
}

// FUNCTION: HGSIM 0x10046f7f
int __cdecl Sim_10046f7f(const char *name, const void *data)
{
    int32_t h = (int32_t)Sim_10109980((uint32_t)(uintptr_t)Sim_100474db(name), 0x8101, 0x80);

    if (h == -1)
        return -1;
    W8U_WRITE_1004((uint32_t)h, data, 0x1d4);
    W8U_CLOSE_1004((uint32_t)h);
    return 0;
}

// FUNCTION: HGSIM 0x10046fe5
int __cdecl Sim_10046fe5(const char *name, uint8_t **out)
{
    uint32_t size, buffer;      /* [ebp-4], [ebp-0xc] */
    int32_t h;

    *out = SIM_API(W8u4HeapAllocFn, 0x10ac5658)(G32(0x1017b8f8), 9, 0x17);
    h = File_ReadAll(Sim_100474db(name), &size, &buffer, 0);
    if (h != -1)
        Mem_Move(*out, (const void *)(uintptr_t)buffer, size);
    else if (*out == 0)
        return -1;
    W8U_CLOSE_1004((uint32_t)h);
    if (G32(0x1015ec58) != 0) {
        (*out)[5] = 2;
        (*out)[1] = 0;
    } else {
        U32(*out, 0xf) = 0;
        U32(*out, 0xb) = 0;
        U32(*out, 0x13) = 0;
        (*out)[9] = 1;
        (*out)[2] = 1;
        (*out)[3] = 1;
        (*out)[4] = 0;
    }
    return 1;
}
