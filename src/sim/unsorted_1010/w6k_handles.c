#include "w6k.h"
#include "w7s.h"
#include "w8A.h"
#include "../util/access.h"
#include "../../platform/plat.h"

typedef int (__stdcall *W6kSetStdHandleFn)(uint32_t which, uint32_t handle);

#define IAT_SETSTDHANDLE 0x10ac56d4u

#define ADDR(p) ((uint32_t)(uintptr_t)(p))
#define BLOCK(i) (SIM_PTR(u32a, 0x10ac3720)[(i)])   /* 64 pointers to blocks of 32 eight-byte slots */
#define SLOT_COUNT G32(0x10ac385c)

/* SetStdHandle for slots 0, 1 and 2 (STD_INPUT/OUTPUT/ERROR_HANDLE) while the dword at 0x1017e900 is 1. */
static void set_std(uint32_t fh, uint32_t value)
{
    if (G32(0x1017e900) != 1)
        return;
    if (fh == 0)
        SIM_API(W6kSetStdHandleFn, IAT_SETSTDHANDLE)((uint32_t)-10, value);
    else if (fh == 1)
        SIM_API(W6kSetStdHandleFn, IAT_SETSTDHANDLE)((uint32_t)-11, value);
    else if (fh == 2)
        SIM_API(W6kSetStdHandleFn, IAT_SETSTDHANDLE)((uint32_t)-12, value);
}

// FUNCTION: HGSIM 0x10109680
int32_t __cdecl Sim_10109680(void)
{
    int32_t r = -1;
    uint32_t blk;
    uint32_t e;
    int i;

    for (i = 0; i < 64; i++) {
        blk = BLOCK(i);
        if (blk == 0) {
            blk = ADDR(W6K_FN_100fef30(0x100));
            if (blk != 0) {
                BLOCK(i) = blk;
                SLOT_COUNT += 0x20;
                for (e = blk; e < BLOCK(i) + 0x100; e += 8) {
                    U8(e, 4) = 0;
                    U32(e, 0) = 0xffffffffu;
                    U8(e, 5) = 0xa;
                }
                r = i << 5;
            }
            return r;
        }
        for (e = blk; e < blk + 0x100; e += 8) {
            if ((U8(e, 4) & 1) == 0) {
                U32(e, 0) = 0xffffffffu;
                r = ((int32_t)(e - BLOCK(i)) >> 3) + i * 32;
                break;
            }
        }
        if (r != -1)
            return r;
    }
    return r;
}

// FUNCTION: HGSIM 0x10109740
int32_t __cdecl Sim_10109740(uint32_t fh, uint32_t value)
{
    int32_t blk = (int32_t)fh >> 5;
    uint32_t off = (fh & 0x1f) * 8;

    if (fh < SLOT_COUNT && U32(BLOCK(blk), off) == 0xffffffffu) {
        set_std(fh, value);
        U32(BLOCK(blk), off) = value;
        return 0;
    }
    G32(0x1017e888) = 9;
    G32(0x1017e88c) = 0;
    return -1;
}

// FUNCTION: HGSIM 0x101097f0
int32_t __cdecl Sim_101097f0(uint32_t fh)
{
    int32_t blk = (int32_t)fh >> 5;
    uint32_t off = (fh & 0x1f) * 8;

    if (fh < SLOT_COUNT && (U8(BLOCK(blk), off + 4) & 1) != 0 && U32(BLOCK(blk), off) != 0xffffffffu) {
        set_std(fh, 0);
        U32(BLOCK(blk), off) = 0xffffffffu;
        return 0;
    }
    G32(0x1017e888) = 9;
    G32(0x1017e88c) = 0;
    return -1;
}

// FUNCTION: HGSIM 0x10109980
uint32_t __cdecl Sim_10109980(uint32_t a, uint32_t b, uint32_t c)
{
    return (uint32_t)Crt_SOpen((const char *)(uintptr_t)a, b, 0x40, c);
}

/*
 * _sopen (C runtime replacement, files: converted from the original, which called Windows; the file itself is
 * opened with Plat_FileCreate now). oflag: 3 access (0 read, 1 write, 2 both), 8 append, 0x10 / 0x20 random /
 * sequential, 0x40 temporary, 0x80 not inherited, 0x100 create, 0x200 truncate, 0x400 exclusive, 0x1000 short-lived,
 * 0x4000 text, 0x8000 binary (else the default mode [0x1017ed18]). shflag 0x10..0x40: the share modes. pmode bit
 * 0x80 (writable, less the umask [0x1017e890]) for a created file. Returns the handle number, or -1 with the
 * error dwords set. A text file opened for reading and writing loses a final Ctrl-Z, as the original.
 */
// FUNCTION: HGSIM 0x101099a0
int32_t __cdecl Crt_SOpen(const char *path, uint32_t oflag, uint32_t shflag, uint32_t pmode)
{
    uint32_t sa[3];             /* SECURITY_ATTRIBUTES {size, descriptor, inherit} */
    uint8_t flags;
    uint32_t access, share, create, attrib, h, type, slot;
    int32_t fh, pos;
    uint8_t ch;

    sa[0] = 0xc;
    sa[1] = 0;
    if (oflag & 0x80) {
        sa[2] = 0;
        flags = 0x10;           /* not inherited */
    } else {
        sa[2] = 1;
        flags = 0;
    }
    if (!(oflag & 0x8000) && ((oflag & 0x4000) || G32(0x1017ed18) != 0x8000))
        flags |= 0x80;          /* text */
    switch (oflag & 3) {
    case 0: access = 0x80000000u; break;
    case 1: access = 0x40000000u; break;
    case 2: access = 0xc0000000u; break;
    default: goto invalid;
    }
    switch (shflag) {
    case 0x10: share = 0; break;
    case 0x20: share = 1; break;
    case 0x30: share = 2; break;
    case 0x40: share = 3; break;
    default: goto invalid;
    }
    switch (oflag & 0x700) {
    case 0x000: case 0x400: create = 3; break;      /* open existing */
    case 0x100: create = 4; break;                  /* open always */
    case 0x200: case 0x600: create = 5; break;      /* truncate existing */
    case 0x300: create = 2; break;                  /* create always */
    case 0x500: case 0x700: create = 1; break;      /* create new */
    default: goto invalid;
    }
    attrib = 0x80;
    if ((oflag & 0x100) && !(pmode & ~G32(0x1017e890) & 0x80))
        attrib = 1;             /* read only */
    if (oflag & 0x40) {
        attrib |= 0x4000000;    /* delete on close */
        access |= 0x10000;
    }
    if (oflag & 0x1000)
        attrib |= 0x100;
    if (oflag & 0x20)
        attrib |= 0x8000000;
    else if (oflag & 0x10)
        attrib |= 0x10000000;
    fh = Sim_10109680();
    if (fh == -1) {
        G32(0x1017e888) = 0x18;
        G32(0x1017e88c) = 0;
        return -1;
    }
    h = Plat_FileCreate(path, access, share, sa, create, attrib, 0);
    if (h == PLAT_FILE_INVALID) {
        Sim_10105100(Plat_FileLastError());
        return -1;
    }
    type = Plat_FileType(h);
    if (type == 0) {
        Plat_FileClose(h);
        Sim_10105100(Plat_FileLastError());
        return -1;
    }
    if (type == 2)
        flags |= 0x40;          /* a device */
    else if (type == 3)
        flags |= 8;             /* a pipe */
    Sim_10109740((uint32_t)fh, h);
    flags |= 1;                 /* open */
    slot = G32(0x10ac3720 + 4u * (uint32_t)(fh >> 5)) + ((uint32_t)fh & 0x1f) * 8u;
    U8(slot, 4) = flags;
    if (!(flags & 0x48) && (flags & 0x80) && (oflag & 2)) {
        pos = Crt_LSeek((uint32_t)fh, -1, 2);
        if (pos == -1) {
            if (G32(0x1017e88c) != 0x83) {
                Crt_Close((uint32_t)fh);
                return -1;
            }
        } else {
            ch = 0;
            /* the original's test: a read answering 0 (not 1) and a Ctrl-Z byte */
            if (Crt_Read((uint32_t)fh, &ch, 1) == 0 && ch == 0x1a && Crt_ChSize((uint32_t)fh, pos) == -1) {
                Crt_Close((uint32_t)fh);
                return -1;
            }
            if (Crt_LSeek((uint32_t)fh, 0, 0) == -1) {
                Crt_Close((uint32_t)fh);
                return -1;
            }
        }
    }
    if (!(flags & 0x48) && (oflag & 8))
        U8(G32(0x10ac3720 + 4u * (uint32_t)(fh >> 5)) + ((uint32_t)fh & 0x1f) * 8u, 4) |= 0x20;   /* append */
    return fh;

invalid:
    G32(0x1017e88c) = 0;
    G32(0x1017e888) = 0x16;
    return -1;
}
