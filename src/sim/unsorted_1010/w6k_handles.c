#include "w6k.h"
#include "../util/access.h"

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
    return W6K_FN_101099a0(a, b, 0x40, c);
}
