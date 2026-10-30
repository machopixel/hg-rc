#include <string.h>
#include "w7s.h"
#include "w6k.h"
#include "../unsorted_100f/w2e.h"
#include "../unsorted_100f/w6i.h"
#include "../util/access.h"

#define ADDR(p) ((uint32_t)(uintptr_t)(p))

typedef int(__stdcall *W7sWideCharToMultiByteFn)(uint32_t cp, uint32_t flags, const uint16_t *src, int32_t count,
                                                 char *dst, int32_t dstCount, const char *defChar, int32_t *used);
typedef uint32_t(__stdcall *W7sGetModuleFileNameAFn)(uint32_t module, char *name, uint32_t size);
typedef void *(__stdcall *W7sNoArgPtrFn)(void);
typedef int(__stdcall *W7sFreeEnvFn)(void *block);
typedef uint32_t(__stdcall *W7sGetStdHandleFn)(uint32_t which);
typedef int(__stdcall *W7sWriteFileFn)(uint32_t handle, const char *data, uint32_t size, uint32_t *written,
                                       void *overlapped);

#define WIDE_TO_MB SIM_API(W7sWideCharToMultiByteFn, 0x10ac5670)
#define GET_MODULE_FILE_NAME SIM_API(W7sGetModuleFileNameAFn, 0x10ac5654)

// FUNCTION: HGSIM 0x10107910
int32_t __cdecl Sim_10107910(void)
{
    uint32_t *list = (uint32_t *)(uintptr_t)G32(0x1017e8b8);
    int32_t n;
    char *text;

    if (list[0] == 0)
        return 0;
    do {
        n = WIDE_TO_MB(1, 0, (const uint16_t *)(uintptr_t)list[0], -1, 0, 0, 0, 0);
        if (n == 0)
            return -1;
        text = Heap_Malloc((uint32_t)n);
        if (text == 0)
            return -1;
        if (WIDE_TO_MB(1, 0, (const uint16_t *)(uintptr_t)list[0], -1, text, n, 0, 0) == 0)
            return -1;
        W7S_FN_1010b2a0(text, 0);
        list++;
    } while (list[0] != 0);
    return 0;
}

// FUNCTION: HGSIM 0x10107990
void __cdecl Sim_10107990(void)
{
    const char *p = (const char *)(uintptr_t)G32(0x1017e8f4);
    uint32_t count = 0;
    uint32_t *slot;

    while (*p != 0) {
        if (*p != '=')
            count++;
        p += strlen(p) + 1;
    }
    slot = Heap_Malloc(count * 4u + 4u);
    G32(0x1017e8b0) = ADDR(slot);
    if (slot == 0)
        W7S_FN_101009b0(9);
    p = (const char *)(uintptr_t)G32(0x1017e8f4);
    while (*p != 0) {
        uint32_t size = (uint32_t)strlen(p) + 1u;

        if (*p != '=') {
            *slot = ADDR(Heap_Malloc(size));
            if (*slot == 0)
                W7S_FN_101009b0(9);
            memcpy((void *)(uintptr_t)*slot, p, strlen(p) + 1);
            slot++;
        }
        p += size;
    }
    Heap_FreeBlock(G32(0x1017e8f4));
    G32(0x1017e8f4) = 0;
    *slot = 0;
}

// FUNCTION: HGSIM 0x10107a80
int32_t __cdecl Sim_10107a80(void)
{
    int32_t nargs;
    int32_t nchars;
    const uint8_t *cmd;
    uint8_t *block;

    GET_MODULE_FILE_NAME(0, SIM_PTR(char, 0x1017e9b8), 0x104);
    G32(0x1017e8c0) = ADDR(SIM_PTR(char, 0x1017e9b8));
    cmd = (const uint8_t *)(uintptr_t)G32(0x10ac4ba4);
    if (cmd[0] == 0)
        cmd = SIM_PTR(const uint8_t, 0x1017e9b8);
    Sim_10107b20(cmd, 0, 0, &nargs, &nchars);
    block = Heap_Malloc((uint32_t)nchars + (uint32_t)nargs * 4u);
    if (block == 0)
        W7S_FN_101009b0(8);
    Sim_10107b20(cmd, (uint32_t *)block, block + (uint32_t)nargs * 4u, &nargs, &nchars);
    G32(0x1017e8a8) = ADDR(block);
    G32(0x1017e8a4) = (uint32_t)(nargs - 1);
    return nargs - 1;
}

/* Number of bytes of a block of 0-ended texts ended by an empty one, with the last 0. */
static uint32_t multi_size_a(const char *p)
{
    const char *e = p;

    if (*e != 0) {
        do {
            e++;
            while (*e != 0)
                e++;
            e++;
        } while (*e != 0);
    }
    return (uint32_t)(e - p) + 1u;
}

static uint32_t multi_size_w(const uint16_t *p)
{
    const uint16_t *e = p;

    if (*e != 0) {
        do {
            e++;
            while (*e != 0)
                e++;
            e++;
        } while (*e != 0);
    }
    return (uint32_t)((int32_t)((const char *)e - (const char *)p) >> 1) + 1u;
}

// FUNCTION: HGSIM 0x10108220
char *__cdecl Sim_10108220(void)
{
    uint32_t how = G32(0x1017ebe4);
    uint16_t *wenv = 0;
    char *aenv = 0;

    if (how == 0) {
        wenv = SIM_API(W7sNoArgPtrFn, 0x10ac56cc)();
        if (wenv != 0)
            how = 1;
        else {
            aenv = SIM_API(W7sNoArgPtrFn, 0x10ac56c8)();
            if (aenv == 0)
                return 0;
            how = 2;
        }
        G32(0x1017ebe4) = how;
    }
    if (how == 1) {
        int32_t n;
        int32_t size;
        char *out;

        if (wenv == 0) {
            wenv = SIM_API(W7sNoArgPtrFn, 0x10ac56cc)();
            if (wenv == 0)
                return 0;
        }
        n = (int32_t)multi_size_w(wenv);
        size = WIDE_TO_MB(0, 0, wenv, n, 0, 0, 0, 0);
        if (size == 0 || (out = Heap_Malloc((uint32_t)size)) == 0) {
            SIM_API(W7sFreeEnvFn, 0x10ac56c4)(wenv);
            return 0;
        }
        if (WIDE_TO_MB(0, 0, wenv, n, out, size, 0, 0) == 0) {
            Heap_FreeBlock(ADDR(out));
            out = 0;
        }
        SIM_API(W7sFreeEnvFn, 0x10ac56c4)(wenv);
        return out;
    }
    if (how == 2) {
        uint32_t size;
        char *out;

        if (aenv == 0) {
            aenv = SIM_API(W7sNoArgPtrFn, 0x10ac56c8)();
            if (aenv == 0)
                return 0;
        }
        size = multi_size_a(aenv);
        out = Heap_Malloc(size);
        if (out == 0) {
            SIM_API(W7sFreeEnvFn, 0x10ac56c0)(aenv);
            return 0;
        }
        memcpy(out, aenv, size);
        SIM_API(W7sFreeEnvFn, 0x10ac56c0)(aenv);
        return out;
    }
    return 0;
}

// FUNCTION: HGSIM 0x10108430
void __cdecl Sim_10108430(uint32_t code)
{
    /* the original keeps the message at esp+0x14 and the program name right after it (esp+0xb4) */
    char frame[0xa0 + 0x104];
    char *msg = frame;
    char *prog = frame + 0xa0;
    char *name;
    uint32_t i;
    uint32_t f;
    const char *text;

    for (i = 0; 0x1015bf58 + i * 8 < 0x1015bfe8; i++)
        if (code == G32(0x1015bf58 + i * 8))
            break;
    if (code != G32(0x1015bf58 + i * 8))
        return;
    f = G32(0x1017e8fc);
    if (f == 1 || (f == 0 && G32(0x1017e900) == 1)) {
        uint32_t written;
        uint32_t handle;

        if (G32(0x10ac3720) == 0 || (handle = U32(G32(0x10ac3720), 0x10)) == 0xffffffffu)
            handle = SIM_API(W7sGetStdHandleFn, 0x10ac5698)(0xfffffff4u);
        text = (const char *)(uintptr_t)G32(0x1015bf5c + i * 8);
        SIM_API(W7sWriteFileFn, 0x10ac568c)(handle, text, (uint32_t)strlen(text), &written, 0);
        return;
    }
    if (code == 0xfc)
        return;
    if (GET_MODULE_FILE_NAME(0, prog, 0x104) == 0)
        memcpy(prog, SIM_PTR(const char, 0x1012bee4), 23);     /* "<program name unknown>" */
    name = prog;
    if (strlen(prog) + 1 > 0x3c) {
        name = prog + strlen(prog) - 0x3b;
        Str_CopyN(name, SIM_PTR(const char, 0x1012bde8), 3);  /* "..." */
    }
    memcpy(msg, SIM_PTR(const char, 0x1012c420), 26);       /* "Runtime Error!\n\nProgram: " */
    memcpy(msg + strlen(msg), name, strlen(name) + 1);
    memcpy(msg + strlen(msg), SIM_PTR(const char, 0x1012bde0), strlen(SIM_PTR(const char, 0x1012bde0)) + 1);
    text = (const char *)(uintptr_t)G32(0x1015bf5c + i * 8);
    memcpy(msg + strlen(msg), text, strlen(text) + 1);
    Sim_10107840(msg, SIM_PTR(const char, 0x1012be9c), 0x12010);
}

// FUNCTION: HGSIM 0x1010b530
char **__cdecl Sim_1010b530(char **list)
{
    uint32_t n = 0;
    char **out;
    char **q;

    if (list == 0)
        return 0;
    while (list[n] != 0)
        n++;
    out = Heap_Malloc(n * 4u + 4u);
    if (out == 0)
        W7S_FN_101009b0(9);
    q = out;
    while (*list != 0)
        *q++ = Sim_1010c9f0(*list++);
    *q = 0;
    return out;
}
