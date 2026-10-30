#include "w5y.h"
#include "../unsorted_1001/w3t.h"
#include "../unsorted_1001/w5y.h"
#include "../util/access.h"

// FUNCTION: HGSIM 0x100c03d8
void __cdecl Sim_100c03d8(void *obj)
{
    float x;

    Sim_1001403a(obj, (short)U16(obj, 0x1aa));
    U32(U32(obj, 0x54), 8) = 0;
    x = Sim_10015266(obj);
    SIM_FN(int(__cdecl *)(void *, float), 0x1000ae00)(obj, x);
}

// FUNCTION: HGSIM 0x100c0d9d
int __cdecl Sim_100c0d9d(void *obj, short value)
{
    float x;

    Sim_1001403a(obj, value);
    x = Sim_10015266(obj);
    SIM_FN(int(__cdecl *)(void *, float), 0x1000ae00)(obj, x);
    return 0;
}

// FUNCTION: HGSIM 0x100c6f39
int __cdecl Sim_100c6f39(void *obj, int idx)
{
    uint8_t *record;

    if (idx == -1)
        return -1;
    record = BYTES(obj) + (uint32_t)idx * 12u + 4;
    if (SIM_FN(int(__cdecl *)(uint32_t, uint32_t, int), 0x100feb20)(U32(obj, 0), U32(record, 4), 0) != 0)
        return -1;
    return S32(record, 8);
}

// FUNCTION: HGSIM 0x100c6f82
int __cdecl Sim_100c6f82(void *obj, const char *name)
{
    int idx = SIM_FN(int(__cdecl *)(void *, const char *), 0x100c6eae)(obj, name);

    if (idx == -1)
        return -1;
    return Sim_100c6f39(obj, idx);
}

// FUNCTION: HGSIM 0x100cbe13
void __cdecl Sim_100cbe13(uint32_t arg)
{
    SIM_FN(void(__cdecl *)(uint32_t, uint32_t, int), 0x100ca670)(PTRTAB(G32(0x1017b8c8)), arg, 2);
}

// FUNCTION: HGSIM 0x100cbe3c
void __cdecl Sim_100cbe3c(uint32_t arg)
{
    SIM_FN(void(__cdecl *)(uint32_t, uint32_t, int), 0x100ca670)(PTRTAB(G32(0x1017b8c8)), arg, 0x20008);
}
