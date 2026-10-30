#include "w2l.h"
#include "../util/access.h"

// FUNCTION: HGSIM 0x100c3eb5
int __cdecl Sim_100c3eb5(const void *obj)
{
    uint32_t q;

    if (U32(obj, 0x4f4) == 0)
        return 0;
    q = U32(obj, 0);
    if ((int8_t)U8(U32(q, 0x54), 0x59) == 1)
        return 0;
    if (U32(q, 0x264) != 0)
        return 0;
    if (S16(q, 0x1f8) == 10)
        return 0;
    return 1;
}

// FUNCTION: HGSIM 0x100c4898
void __cdecl Sim_100c4898(void *obj)
{
    if (obj == 0 || U32(obj, 0x54) == 0)
        return;
    U32(U32(obj, 0x54), 8) = 0;
    U8(U32(obj, 0x54), 0x54) = 1;
}

// FUNCTION: HGSIM 0x100c4966
int __cdecl Sim_100c4966(void *obj, int *out)
{
    int32_t start;

    if (obj == 0) {
        *out = -1;
        return -1;
    }
    start = S32(obj, 0x208);
    do {
        S32(obj, 0x208) = (int32_t)((uint32_t)S32(obj, 0x208) + 1u);
        if (S32(obj, 0x208) == 3)
            S32(obj, 0x208) = 0;
    } while (S32(obj, 0x20c + (uint32_t)S32(obj, 0x208) * 4u) == -1 && S32(obj, 0x208) != start);
    if (S32(obj, 0x208) == start) {
        *out = -1;
        return -1;
    }
    *out = S32(obj, 0x208);
    return S32(obj, 0x20c + (uint32_t)S32(obj, 0x208) * 4u);
}

// FUNCTION: HGSIM 0x100cb9f3
int __cdecl Sim_100cb9f3(void)
{
    uint32_t entry = PTRTAB(GS32(0x1017b8c8));
    uint32_t kind = U32(entry, 0xf0) & 0xf00;
    uint32_t low = U32(entry, 0xf0) & 0xff;

    if (kind != 0x400)
        low = 0xffffffffu;
    return (int)low;
}
