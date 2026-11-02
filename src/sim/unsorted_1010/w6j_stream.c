#include "w6j.h"
#include "w2k.h"
#include "../util/access.h"
#include "../../platform/plat.h"



// FUNCTION: HGSIM 0x101048e0
int __cdecl Sim_101048e0(int ch)
{
    int has;

    if ((int32_t)G32(0x1015965c) > 1)
        has = W6J_CHARMASK(ch, 4);
    else
        has = U8(G32(0x10159450) + (uint32_t)ch * 2u, 0) & 4;
    if (has == 0)
        ch = (int)(((uint32_t)ch & 0xffffffdfu) - 7u);
    return ch;
}

// FUNCTION: HGSIM 0x10104920
int __cdecl Sim_10104920(void *stream)
{
    uint32_t left = U32(stream, 4) - 1u;
    uint32_t p;
    uint8_t b;

    U32(stream, 4) = left;
    if ((int32_t)left < 0)
        return W6J_FILL(stream);
    p = U32(stream, 0);
    b = U8(p, 0);
    U32(stream, 0) = p + 1u;
    return b;
}

// FUNCTION: HGSIM 0x10104950
int __cdecl Sim_10104950(int ch, void *stream)
{
    if (ch != -1)
        return W6J_PUTBACK(ch, stream);
    return ch;
}

// FUNCTION: HGSIM 0x10104aa0
void __cdecl Sim_10104aa0(void *stream)
{
    uint32_t flags = U32(stream, 0xc);

    if ((flags & 0x83u) != 0 && (flags & 8u) != 0) {
        W6J_FREE((void *)(uintptr_t)U32(stream, 8));
        U32(stream, 0xc) &= 0xfffffbf7u;
        U32(stream, 0) = 0;
        U32(stream, 8) = 0;
        U32(stream, 4) = 0;
    }
}

// FUNCTION: HGSIM 0x10104ae0
int __cdecl Sim_10104ae0(void *stream)
{
    uint32_t slot;
    uint32_t buf;

    if (Sim_10107080(U32(stream, 0x10)) == 0)
        return 0;
    if ((uint8_t *)stream == SIM_PTR(uint8_t, 0x10159b68))
        slot = 0;
    else if ((uint8_t *)stream == SIM_PTR(uint8_t, 0x10159b88))
        slot = 1;
    else
        return 0;
    G32(0x1017e964) += 1u;
    if ((U32(stream, 0xc) & 0x10cu) != 0)
        return 0;
    if (G32(0x1017e930 + slot * 4u) == 0) {
        G32(0x1017e930 + slot * 4u) = (uint32_t)(uintptr_t)W6J_MALLOC(0x1000);
        if (G32(0x1017e930 + slot * 4u) == 0)
            return 0;
    }
    buf = G32(0x1017e930 + slot * 4u);
    U32(stream, 0x18) = 0x1000;
    U32(stream, 8) = buf;
    U32(stream, 0) = buf;
    U32(stream, 4) = 0x1000;
    U32(stream, 0xc) |= 0x1102u;
    return 1;
}

// FUNCTION: HGSIM 0x10104b80
void __cdecl Sim_10104b80(int release, void *stream)
{
    if (release != 0) {
        if ((U32(stream, 0xc) & 0x1000u) != 0) {
            W6J_FLUSH(stream);
            U32(stream, 0xc) &= 0xffffeeffu;
            U32(stream, 0x18) = 0;
            U32(stream, 0) = 0;
            U32(stream, 8) = 0;
        }
    } else if ((U32(stream, 0xc) & 0x1000u) != 0) {
        W6J_FLUSH(stream);
    }
}

// FUNCTION: HGSIM 0x101056f0
int __cdecl Sim_101056f0(uint32_t fd)
{
    uint32_t block;
    uint32_t err;

    if (fd < G32(0x10ac385c)) {
        block = G32(0x10ac3720u + (uint32_t)((int32_t)fd >> 5) * 4u);
        if ((U8(block, (fd & 0x1fu) * 8u + 4u) & 1) != 0) {
            if (Plat_FileFlush(W6J_OSHANDLE(fd)) != 0)
                err = 0;
            else
                err = Plat_FileLastError();     /* C runtime replacement, files */
            if (err == 0)
                return 0;
            G32(0x1017e88c) = err;
        }
    }
    G32(0x1017e888) = 9;
    return -1;
}

// FUNCTION: HGSIM 0x10105fb0
void __cdecl Sim_10105fb0(void)
{
    uint32_t a;
    uint32_t block;

    for (a = 0x10ac3720u; a < 0x10ac3820u; a += 4u) {
        block = *SIM_PTR(u32a, a);
        if (block != 0) {
            W6J_FREE((void *)(uintptr_t)block);
            *SIM_PTR(u32a, a) = 0;
        }
    }
}
