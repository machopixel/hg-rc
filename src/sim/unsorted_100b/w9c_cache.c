#include "w9c.h"
#include "w3o.h"
#include "w7p.h"
#include "w8y.h"
#include "../unsorted_100f/w6h.h"
#include "../unsorted_100f/w6i.h"
#include "../unsorted_100f/w7q.h"
#include "../unsorted_100f/w8A.h"
#include "../unsorted_100f/w8z.h"
#include "../unsorted_1010/w7s.h"
#include "../util/access.h"

/* Not converted: _assert(expression, file, line) of the DLL's C library. */
#define W9CB_CALL_100ff860 SIM_FN(void(__cdecl *)(const char *, const char *, int), 0x100ff860)
/* Converted, but its hook is switched off on purpose: the pack-file loader (name, extension, out size). */
#define W9CB_CALL_100bca49 SIM_FN(uint32_t(__cdecl *)(const char *, const char *, uint32_t *), 0x100bca49)
/* The type comparison the original passes by address (its C twin is Sim_100b971d). */
#define W9CB_CMP_100b971d SIM_FN(int(__cdecl *)(uint32_t, uint32_t), 0x100b971d)

/* Reads the whole stream into a new block (asserting "item" at the given line of syscache.c when there is
 * no memory) and closes it; *size takes the length. */
static uint32_t W9cB_ReadWhole(void *f, uint32_t *size, uint32_t expr, uint32_t file, int line)
{
    uint32_t data;

    Crt_FSeek((uint8_t *)f, 0, 2);
    *size = (uint32_t)Crt_FTell(f);
    Crt_FSeek((uint8_t *)f, 0, 0);
    data = (uint32_t)(uintptr_t)Heap_Malloc(*size);
    if (data == 0)
        W9CB_CALL_100ff860(SIM_PTR(const char, expr), SIM_PTR(const char, file), line);
    Crt_FRead((void *)(uintptr_t)data, 1, *size, (uint8_t *)f);
    Crt_FClose(f);
    return data;
}

/* Lets the caller's fixup replace the block, notes `arg` in the type record and files the block under
 * (name, ext). The original leaves `replaced` uninitialised when there is no fixup; it is 0 here. */
static uint32_t W9cB_Store(uint32_t data, const char *name, const char *ext, W9cItemFixupFn fixup, uint32_t arg,
                           uint32_t *size)
{
    uint32_t replaced = 0;
    uint32_t type;

    if (fixup != 0)
        replaced = fixup(data, size);
    type = Sim_100bda9f((void *)(uintptr_t)G32(0x1017d604), (uint32_t)(uintptr_t)ext, W9CB_CMP_100b971d);
    U32(type, 0x20) = arg;
    if (replaced != 0) {
        if (data != 0)
            Heap_FreeBlock(data);
        data = replaced;
    }
    Sim_100b9b3f(data, (const uint32_t *)name, ext, *size);
    return data;
}

// FUNCTION: HGSIM 0x100bb405
uint32_t __cdecl Cache_LoadItem(char *name, char *ext, W9cItemFixupFn fixup, uint32_t arg)
{
    char path[0x100];
    uint32_t size;
    uint32_t data;
    void *f;

    Crt_StrUpper(name);
    Crt_StrUpper(ext);
    data = Sim_100baf3f((uint32_t *)name, (uint32_t)(uintptr_t)ext, &size);
    if (data != 0)
        return data;
    Crt_SPrintf(path, SIM_PTR(const char, 0x10151234), name, ext);           /* "%s.%s" */
    f = Sim_100fd850(path, SIM_PTR(const char, 0x1015123c));                  /* "rb" */
    if (f != 0) {
        data = W9cB_ReadWhole(f, &size, 0x10151264, 0x10151240, 0x598);
        return W9cB_Store(data, name, ext, fixup, arg, &size);
    }
    if (G32(0x1017d5fc) != 0) {
        data = W9CB_CALL_100bca49(name, ext, &size);
        if (data == 0)
            return 0;
        return W9cB_Store(data, name, ext, fixup, arg, &size);
    }
    f = Sim_100bae0a(name, ext);
    if (f == 0)
        return 0;
    data = W9cB_ReadWhole(f, &size, 0x10151290, 0x1015126c, 0x5cd);
    return W9cB_Store(data, name, ext, fixup, arg, &size);
}
