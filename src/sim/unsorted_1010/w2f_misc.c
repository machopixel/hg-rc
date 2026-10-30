#include "w2f.h"
#include "../util/access.h"

// FUNCTION: HGSIM 0x101003a0
int __cdecl Crt_Rand(void)
{
    uint32_t seed = G32(0x101596f8) * 214013u + 2531011u;

    G32(0x101596f8) = seed;
    return (int)((seed >> 16) & 0x7fff);
}

// FUNCTION: HGSIM 0x1010cd90
int __cdecl Sim_1010cd90(uint32_t fd, int mode)
{
    uint32_t group;
    uint8_t *entry;
    uint8_t flags;
    uint32_t old;

    if (fd >= G32(0x10ac385c)) {
        G32(0x1017e888) = 9;
        return -1;
    }
    group = G32(0x10ac3720 + (uint32_t)((int32_t)fd >> 5) * 4);
    entry = BYTES(group) + (fd & 0x1f) * 8 + 4;
    flags = *entry;
    if ((flags & 1) == 0) {
        G32(0x1017e888) = 9;
        return -1;
    }
    old = flags & 0x80;
    if (mode == 0x8000) {
        flags &= 0x7f;
    } else if (mode == 0x4000) {
        flags |= 0x80;
    } else {
        G32(0x1017e888) = 0x16;
        return -1;
    }
    *entry = flags;
    return old != 0 ? 0x4000 : 0x8000;
}
