#include "w2h.h"
#include "../unsorted_1001/w2h.h"
#include "../util/access.h"
#include "w3p.h"

// FUNCTION: HGSIM 0x10040721
void __cdecl Sim_10040721(void *obj, const void *src)
{
    int i;

    U16(obj, 0x198) = U16(src, 8);
    for (i = 0; i < 2; i++)
        U32(obj, 0x180 + (uint32_t)i * 4) = U32(src, (uint32_t)i * 4);
    U16(obj, 0x1a0) = U8(src, 0xa);
    U16(obj, 0x1a2) = U16(src, 0xb);
    U16(obj, 0x1a4) = U16(src, 0xd);
    S16(obj, 0x1a6) = (int8_t)U8(src, 0xf);
    U16(obj, 0x1a8) = U16(src, 0x10);
    U16(obj, 0x1aa) = U16(src, 0x12);
    U32(obj, 0x1ae) = U16(src, 0x14);
    U16(obj, 0x1f8) = 0xffff;
    U16(obj, 0x1fa) = 0xffff;
    U32(obj, 0x1fc) = 0xffffffffu;
    U32(obj, 0x23e) = U32(src, 0x16);
    U32(obj, 0x242) = U32(src, 0x1a);
    Sim_100119a7(obj);
    U32(obj, 0xf0) = U16(obj, 0x1aa);
}

// FUNCTION: HGSIM 0x1004094e
void __cdecl Sim_1004094e(const void *record, int32_t index)
{
    uint32_t rec;

#ifndef NETG_MUTATE /* the checker's negative check only: no guard */
    if (g_w34NetOver != 0)          /* 1.2 (0x1003458e): ignored once the game is over */
        return;
#endif

    if (index < 0 || index >= GS32(0x1015dc64))
        return;
    rec = PTRTAB(index);
    if (rec == 0 || S32(rec, 0x14) != 3 || U32(rec, 0xc) == G32(0x1017b8c8))
        return;
    Sim_10040721((void *)(uintptr_t)rec, (const uint8_t *)record + 3);
}
