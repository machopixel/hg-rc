#include "w3t.h"
#include "../util/access.h"
#include "../unsorted_100f/w2e.h"

/* The cache loader (thunk 0x10003413, not converted): (file name, constant, 0) -> data or NULL. */
#define LOAD_CACHED(name, arg) SIM_FN(void *(__cdecl *)(void *, void *, int), 0x10003413)((name), (arg), 0)

// FUNCTION: HGSIM 0x1001b705
int __cdecl Sim_1001b705(const char *name, void *obj)
{
    uint32_t image, p;
    int i;

    if (obj == 0 || name == 0)
        return 1;
    Str_Copy((char *)BYTES(obj) + 4, name);
    image = (uint32_t)(uintptr_t)LOAD_CACHED(BYTES(obj) + 4, GBYTES(0x10131944));
    U32(obj, 0x18) = image;
    if (image == 0)
        return 1;
    U32(obj, 0x14) = image;
    p = image + 0x10;
    for (i = 0; i < S32(U32(obj, 0x14), 0xc); i++) {
        uint32_t base = (uint32_t)i * 0xcu;

        if (Str_Length((const char *)BYTES(p)) < 8)
            Str_Copy((char *)BYTES(p), (const char *)GBYTES(0x10131948));
        U32(obj, base + 0x1c) = p;
        p += 0x20;
        U32(obj, base + 0x20) = p;
        p = p + (U32(U32(obj, base + 0x20), 0) << 4) + 4;
        U32(obj, base + 0x24) = p;
        p = p + U32(U32(obj, base + 0x24), 0) * 0x14u + 4;
    }
    U32(obj, 0x31c) = p;
    U32(obj, 0) = 1;
    G32(0x1015d18c) = G32(0x1015d18c) + 1;
    return 0;
}

// FUNCTION: HGSIM 0x1001b880
void *__cdecl Sim_1001b880(const char *name)
{
    uint8_t *slot = 0;
    int i;

    for (i = 0; i < 0x80 && slot == 0; i++) {
        if (U32(GBYTES(0x10a1eaa0), (uint32_t)i * 0x320u) == 0)
            slot = GBYTES(0x10a1eaa0) + (uint32_t)i * 0x320u;
    }
    if (slot != 0 && Sim_1001b705(name, slot) == 0)
        return slot;
    return 0;
}
