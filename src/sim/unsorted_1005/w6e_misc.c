#include "w6e.h"
#include "w5a.h"
#include "../unsorted_1004/fns.h"
#include "../unsorted_100a/w5x.h"
#include "../view/w3r.h"
#include "../util/access.h"

// FUNCTION: HGSIM 0x100529a2
int __cdecl Sim_100529a2(uint32_t index, uint32_t *x, uint32_t *y, uint32_t *z)
{
    uint32_t zero = 0;
    uint32_t group, other, column, row;

    if (index >= G32(0x1015dc64))
        return 0;
    group = U32(PTRTAB(index), 8);
    other = (uint32_t)Sim_10052aaf((int)group);
    column = U32(PTRTAB(index), 0x10);
    if (other != index) {
        row = group * 0x74u + column * 4u;
        *(u32a *)x = G32(0x109b8430u + row);
        *(u32a *)y = G32(0x109b8450u + row);
        *(u32a *)z = G32(0x109b8470u + row);
        Mat3x4_TransformPoint(Sim_100aa863((void *)(uintptr_t)U32(PTRTAB(other), 0x48)), (float *)x, (float *)&zero,
                              (float *)y);
    } else {
        *(u32a *)x = U32(PTRTAB(other), 0x58);
        *(u32a *)y = U32(PTRTAB(other), 0x60);
        *(u32a *)z = U32(PTRTAB(other), 0x68);
    }
    return 1;
}

// FUNCTION: HGSIM 0x1005790a
int __cdecl Sim_1005790a(uint32_t unused, char *rec)
{
    (void)unused;
    Sim_100579ad((const uint32_t *)(uintptr_t)U32(G32(0x10144230), 2), (const uint32_t *)(uintptr_t)U32(G32(0x10144230), 6));
    U32(rec, 0x58) = 0;
    U32(rec, 0x5c) = 0x7d;
    W6E_CALL_100fc780(rec, SIM_PTR(char, 0x10143d88));
    W6E_CALL_100fc780(rec + 0xc, (const char *)(uintptr_t)G32(0x101c4fdc));
    W6E_CALL_100fc780(rec + 0x4c, SIM_PTR(char, 0x10143d94));
    U32(rec, 0x60) = 0;
    U32(rec, 0x64) = 0;
    U32(rec, 0x68) = (uint32_t)(uintptr_t)SIM_PTR(void, 0x10165f60);
    U32(rec, 0x6c) = (uint32_t)(uintptr_t)SIM_PTR(void, 0x10143808);
    U32(rec, 0x70) = 0;
    return 0;
}

// FUNCTION: HGSIM 0x1005ada5
int __cdecl Sim_1005ada5(void *obj)
{
    int i;
    int entry;
    char first = (char)U8(U32(obj, 0x88), 0x7c);
    uint32_t saved = 0;

    entry = (first == 'g' || first == 'G') ? 1 : 0;
    for (i = 0; i < 4; i++) {
        if ((int8_t)U8(U32(obj, 0x88), (uint32_t)entry * 0x20u + 0x7c) == 0)
            break;
        W6E_CALL_1005a91a(obj, i, 0);
        if (i != 0)
            G32(0x101c42a0) = saved;
        else
            saved = G32(0x101c42a0);
        entry++;
    }
    return i;
}
