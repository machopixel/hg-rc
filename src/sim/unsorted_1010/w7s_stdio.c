#include <string.h>
#include "w7s.h"
#include "w6j.h"
#include "w6k.h"
#include "../unsorted_100f/w6i.h"
#include "../util/access.h"

#define ADDR(p) ((uint32_t)(uintptr_t)(p))

typedef void(__stdcall *W7sGetStartupInfoAFn)(void *info);
typedef uint32_t(__stdcall *W7sGetFileTypeFn)(uint32_t handle);
typedef uint32_t(__stdcall *W7sGetStdHandleFn)(uint32_t which);
typedef uint32_t(__stdcall *W7sSetHandleCountFn)(uint32_t count);

/* Slot of handle number fh in the handle table (block fh >> 5, eight bytes per slot). */
static uint32_t handle_slot(uint32_t fh)
{
    return G32(0x10ac3720 + (uint32_t)((int32_t)fh >> 5) * 4u) + (fh & 0x1f) * 8u;
}

// FUNCTION: HGSIM 0x10104970
int32_t __cdecl Sim_10104970(int32_t *count, void *stream)
{
    int32_t c;

    ++*count;
    c = Sim_10104920(stream);
    while (Sim_101093f0(c) != 0) {
        ++*count;
        c = Sim_10104920(stream);
    }
    return c;
}

// FUNCTION: HGSIM 0x10104be0
uint8_t *__cdecl Sim_10104be0(const char *name, const char *mode, uint32_t share, uint8_t *stream)
{
    uint32_t oflag;
    uint32_t sflag = G32(0x1017ec20);
    uint32_t more = 1;
    uint32_t seenCommit = 0;
    uint32_t seenAccess = 0;
    int32_t fh;

    switch (mode[0]) {
    case 'r':
        oflag = 0;
        sflag |= 1;
        break;
    case 'w':
        oflag = 0x301;
        sflag |= 2;
        break;
    case 'a':
        oflag = 0x109;
        sflag |= 2;
        break;
    default:
        return 0;
    }
    for (mode++; *mode != 0 && more; mode++) {
        switch (*mode) {
        case '+':
            if (oflag & 2)
                more = 0;
            else {
                oflag = (oflag & ~1u) | 2;
                sflag = (sflag & ~3u) | 0x80;
            }
            break;
        case 'b':
            if (oflag & 0xc000)
                more = 0;
            else
                oflag |= 0x8000;
            break;
        case 't':
            if (oflag & 0xc000)
                more = 0;
            else
                oflag |= 0x4000;
            break;
        case 'c':
            if (seenCommit)
                more = 0;
            else {
                seenCommit = 1;
                sflag |= 0x4000;
            }
            break;
        case 'n':
            if (seenCommit)
                more = 0;
            else {
                seenCommit = 1;
                sflag &= ~0x4000u;
            }
            break;
        case 'S':
            if (seenAccess)
                more = 0;
            else {
                seenAccess = 1;
                oflag |= 0x20;
            }
            break;
        case 'R':
            if (seenAccess)
                more = 0;
            else {
                seenAccess = 1;
                oflag |= 0x10;
            }
            break;
        case 'T':
            if (oflag & 0x1000)
                more = 0;
            else
                oflag |= 0x1000;
            break;
        case 'D':
            if (oflag & 0x40)
                more = 0;
            else
                oflag |= 0x40;
            break;
        default:
            more = 0;
            break;
        }
    }
    fh = W7S_FN_101099a0(name, oflag, share, 0x1a4);
    if (fh < 0)
        return 0;
    G32(0x1017e964)++;
    U32(stream, 0xc) = sflag;
    U32(stream, 4) = 0;
    U32(stream, 0) = 0;
    U32(stream, 8) = 0;
    U32(stream, 0x1c) = 0;
    U32(stream, 0x10) = (uint32_t)fh;
    return stream;
}

// FUNCTION: HGSIM 0x10104db0
uint8_t *__cdecl Sim_10104db0(void)
{
    int32_t n = GS32(0x10ac4ba0);
    uint32_t found = 0;
    int32_t i;

    for (i = 0; i < n; i++) {
        uint32_t s = U32(G32(0x10ac3860), i * 4);

        if (s == 0) {
            U32(G32(0x10ac3860), i * 4) = ADDR(Heap_Malloc(0x20));
            s = U32(G32(0x10ac3860), i * 4);
            if (s != 0)
                found = s;
            break;
        }
        if ((U8(s, 0xc) & 0x83) == 0) {
            found = U32(G32(0x10ac3860), i * 4);
            break;
        }
    }
    if (found != 0) {
        U32(found, 4) = 0;
        U32(found, 0xc) = 0;
        U32(found, 8) = 0;
        U32(found, 0) = 0;
        U32(found, 0x1c) = 0;
        U32(found, 0x10) = 0xffffffffu;
    }
    return BYTES(found);
}

// FUNCTION: HGSIM 0x10105100
void __cdecl Sim_10105100(uint32_t oserr)
{
    uint32_t i;

    G32(0x1017e88c) = oserr;
    for (i = 0; 0x101599e0 + i * 8 < 0x10159b48; i++) {
        if (oserr == G32(0x101599e0 + i * 8)) {
            G32(0x1017e888) = G32(0x101599e4 + i * 8);
            return;
        }
    }
    if (oserr >= 0x13 && oserr <= 0x24)
        G32(0x1017e888) = 0xd;
    else if (oserr >= 0xbc && oserr <= 0xca)
        G32(0x1017e888) = 8;
    else
        G32(0x1017e888) = 0x16;
}

// FUNCTION: HGSIM 0x10105a60
int32_t __cdecl Sim_10105a60(uint8_t *stream)
{
    uint32_t flags = U32(stream, 0xc);
    int32_t got;
    uint32_t p;
    uint8_t c;

    if ((flags & 0x83) == 0 || (flags & 0x40) != 0)
        return -1;
    if (flags & 2) {
        U32(stream, 0xc) = flags | 0x20;
        return -1;
    }
    flags |= 1;
    U32(stream, 0xc) = flags;
    if ((flags & 0x10c) == 0)
        Sim_10107020(stream);
    else
        U32(stream, 0) = U32(stream, 8);
    got = W7S_FN_10105b50(U32(stream, 0x10), U32(stream, 8), U32(stream, 0x18));
    U32(stream, 4) = (uint32_t)got;
    if (got == 0 || got == -1) {
        U32(stream, 4) = 0;
        U32(stream, 0xc) |= got != 0 ? 0x20u : 0x10u;
        return -1;
    }
    flags = U32(stream, 0xc);
    if ((flags & 0x82) == 0) {
        uint32_t fh = U32(stream, 0x10);
        uint32_t slot = fh == 0xffffffffu ? ADDR(SIM_PTR(uint8_t, 0x10159dc8)) : handle_slot(fh);

        if ((U8(slot, 4) & 0x82) == 0x82)
            U32(stream, 0xc) = flags | 0x2000;
    }
    if (U32(stream, 0x18) == 0x200 && (U32(stream, 0xc) & 8) != 0 && (U32(stream, 0xc) & 0x400) == 0)
        U32(stream, 0x18) = 0x1000;
    U32(stream, 4) = (uint32_t)got - 1u;
    p = U32(stream, 0);
    c = U8(p, 0);
    U32(stream, 0) = p + 1;
    return c;
}

/* Marks the 32 slots of a new block unused: handle -1, flags 0, the byte 10. */
static void init_block(uint32_t block, uint32_t tableEntry)
{
    uint32_t q;

    for (q = block; q < G32(tableEntry) + 0x100; q += 8) {
        U8(q, 4) = 0;
        U32(q, 0) = 0xffffffffu;
        U8(q, 5) = 10;
    }
}

// FUNCTION: HGSIM 0x10105db0
uint32_t __cdecl Sim_10105db0(void)
{
    uint32_t info[0x11];        /* STARTUPINFOA: cbReserved2 at +0x32, lpReserved2 at +0x34 */
    uint32_t block;
    uint32_t i;
    uint32_t std;

    block = ADDR(Heap_Malloc(0x100));
    if (block == 0)
        W7S_FN_101009b0(0x1b);
    G32(0x10ac3720) = block;
    G32(0x10ac385c) = 0x20;
    if (block < block + 0x100)
        init_block(block, 0x10ac3720);
    SIM_API(W7sGetStartupInfoAFn, 0x10ac56a0)(info);
    if (U16(info, 0x32) != 0 && U32(info, 0x34) != 0) {
        uint32_t data = U32(info, 0x34);
        int32_t n = S32(data, 0);
        uint32_t flags = data + 4;
        uint32_t handles = flags + (uint32_t)n;

        if (n >= 0x800)
            n = 0x800;
        if (GS32(0x10ac385c) < n) {
            uint32_t entry = 0x10ac3724;

            for (;;) {
                uint32_t b = ADDR(Heap_Malloc(0x100));

                if (b == 0) {
                    n = GS32(0x10ac385c);
                    break;
                }
                G32(entry) = b;
                G32(0x10ac385c) += 0x20;
                if (b < b + 0x100)
                    init_block(b, entry);
                entry += 4;
                if (GS32(0x10ac385c) >= n)
                    break;
            }
        }
        for (i = 0; (int32_t)i < n; i++, flags++, handles += 4) {
            uint32_t h = U32(handles, 0);
            uint8_t f;

            if (h == 0xffffffffu)
                continue;
            f = U8(flags, 0);
            if ((f & 1) == 0)
                continue;
            if ((f & 8) == 0 && SIM_API(W7sGetFileTypeFn, 0x10ac569c)(h) == 0)
                continue;
            {
                uint32_t slot = handle_slot(i);

                U32(slot, 0) = U32(handles, 0);
                U8(slot, 4) = U8(flags, 0);
            }
        }
    }
    for (std = 0; std < 3; std++) {
        uint32_t slot = G32(0x10ac3720) + std * 8;

        if (U32(slot, 0) == 0xffffffffu) {
            uint32_t h;
            uint32_t type;

            U8(slot, 4) = 0x81;
            h = SIM_API(W7sGetStdHandleFn, 0x10ac5698)(std == 0 ? 0xfffffff6u : std == 1 ? 0xfffffff5u : 0xfffffff4u);
            if (h != 0xffffffffu && (type = SIM_API(W7sGetFileTypeFn, 0x10ac569c)(h)) != 0) {
                type &= 0xff;
                U32(slot, 0) = h;
                if (type == 2)
                    U8(slot, 4) |= 0x40;
                else if (type == 3)
                    U8(slot, 4) |= 8;
            } else
                U8(slot, 4) |= 0x40;
        } else
            U8(slot, 4) |= 0x80;
    }
    return SIM_API(W7sSetHandleCountFn, 0x10ac5694)(G32(0x10ac385c));
}

// FUNCTION: HGSIM 0x10109890
uint32_t __cdecl Sim_10109890(uint32_t fh)
{
    if (fh < G32(0x10ac385c)) {
        uint32_t slot = handle_slot(fh);

        if (U8(slot, 4) & 1)
            return U32(slot, 0);
    }
    G32(0x1017e888) = 9;
    G32(0x1017e88c) = 0;
    return 0xffffffffu;
}
