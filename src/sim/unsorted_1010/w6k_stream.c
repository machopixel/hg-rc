#include "w6k.h"
#include "../util/access.h"

typedef int (__stdcall *W6kWriteConsoleAFn)(uint32_t handle, const void *text, uint32_t count, uint32_t *written,
                                            void *reserved);
typedef int (__stdcall *W6kWideCharToMultiByteFn)(uint32_t codepage, uint32_t flags, const uint16_t *wide,
                                                  int32_t nwide, char *out, int32_t nout, const char *defchar,
                                                  int32_t *useddef);

#define IAT_WRITECONSOLEA 0x10ac56f8u
#define IAT_WIDECHARTOMULTIBYTE 0x10ac5670u

#define ADDR(p) ((uint32_t)(uintptr_t)(p))

// FUNCTION: HGSIM 0x10107020
void __cdecl Sim_10107020(uint8_t *stream)
{
    void *block;

    G32(0x1017e964)++;
    block = W6K_FN_100fef30(0x1000);
    U32(stream, 8) = ADDR(block);
    if (block != 0) {
        U32(stream, 0xc) |= 8;
        U32(stream, 0x18) = 0x1000;
        U32(stream, 0) = U32(stream, 8);
        U32(stream, 4) = 0;
    } else {
        U32(stream, 0xc) |= 4;
        U32(stream, 0x18) = 2;
        U32(stream, 8) = ADDR(stream + 0x14);
        U32(stream, 0) = ADDR(stream + 0x14);
        U32(stream, 4) = 0;
    }
}

// FUNCTION: HGSIM 0x101095f0
int32_t __cdecl Sim_101095f0(int32_t c, uint8_t *stream)
{
    uint32_t flags;
    uint32_t p;

    if (c == -1)
        return -1;
    flags = U32(stream, 0xc);
    if ((flags & 1) == 0 && ((flags & 0x80) == 0 || (flags & 2) != 0))
        return -1;
    if (U32(stream, 8) == 0)
        Sim_10107020(stream);
    if (U32(stream, 0) == U32(stream, 8)) {
        if (U32(stream, 4) != 0)
            return -1;
        U32(stream, 0)++;
    }
    if ((U8(stream, 0xc) & 0x40) != 0) {
        p = U32(stream, 0) - 1;
        U32(stream, 0) = p;
        if (U8(p, 0) != (uint8_t)c) {
            U32(stream, 0) = p + 1;
            return -1;
        }
    } else {
        p = U32(stream, 0) - 1;
        U32(stream, 0) = p;
        U8(p, 0) = (uint8_t)c;
    }
    flags = U32(stream, 0xc);
    U32(stream, 4)++;
    U32(stream, 0xc) = (flags & ~0x10u) | 1;
    return c & 0xff;
}

// FUNCTION: HGSIM 0x1010d2f0
int32_t __cdecl Sim_1010d2f0(uint32_t c)
{
    uint8_t ch = (uint8_t)c;
    uint32_t written;
    int32_t handle = GS32(0x1015c494);

    if (handle == -2) {
        W6K_FN_1010d390();
        handle = GS32(0x1015c494);
    }
    if (handle == -1)
        return -1;
    if (!SIM_API(W6kWriteConsoleAFn, IAT_WRITECONSOLEA)((uint32_t)handle, &ch, 1, &written, 0))
        return -1;
    return ch;
}

// FUNCTION: HGSIM 0x10109050
int32_t __cdecl Sim_10109050(char *s, uint32_t wc)
{
    uint16_t wide = (uint16_t)wc;
    int32_t useddef;
    int32_t n;

    if (s == 0)
        return 0;
    if (G32(0x1017e950) == 0) {
        if (wide <= 0xff) {
            *s = (char)wide;
            return 1;
        }
    } else {
        useddef = 0;
        n = SIM_API(W6kWideCharToMultiByteFn, IAT_WIDECHARTOMULTIBYTE)(G32(0x1017e960), 0x220, &wide, 1, s,
                                                                       GS32(0x1015965c), 0, &useddef);
        if (n != 0 && useddef == 0)
            return n;
    }
    G32(0x1017e888) = 0x2a;
    return -1;
}
