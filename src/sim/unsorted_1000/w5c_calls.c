#include "w5c.h"
#include "w2h.h"
#include "../unsorted_1003/w2e.h"
#include "../util/util.h"
#include "../util/access.h"

// FUNCTION: HGSIM 0x10007ed5
void __cdecl Sim_10007ed5(void)
{
    char width[0x2c];
    char line[0x2c];

    SIM_CALL_100fc780(width, SIM_PTR(char, 0x1012dd94), 0x28, 0x28);
    /* the first result is the format of the second */
    SIM_CALL_100fc780(line, width, SIM_PTR(char, 0x1012dc50));
    if (G32(0x10a3ade8) != 0)
        Mono_PutText(line, 0, 1, 0xf);
    if (G32(0x10a3ade8) != 0)
        Mono_PutText(line, 0, 2, 0xf);
    G32(0x1015d03c) = 0;
}

// FUNCTION: HGSIM 0x10007f49
void __cdecl Sim_10007f49(const void *obj)
{
    int32_t a = 0;
    int32_t b = 0;
    int n;

    Sim_100094df(obj, &a, &b);
    G32(0x1015d014) += 1;
    G32(0x1015d00c) += (uint32_t)a;
    G32(0x1015d018) += (uint32_t)b;
    n = SIM_CALL_100097d3(obj);
    G32(0x1015d010) += (uint32_t)n;
}

// FUNCTION: HGSIM 0x10010496
void __cdecl Sim_10010496(int slot, int unused)
{
    int idx;
    (void)unused;
    if (slot >= 8)
        return;
    idx = Sim_10015c61(slot);
    if (idx == -1)
        return;
    if (S32(U32(PTRTAB(idx), 0x28), 0x4f0) != 2)
        return;
    SIM_PTR(u32a, 0x1012ff58)[slot] = 2;
    SIM_CALL_10016117(slot, 0xe);
}

// FUNCTION: HGSIM 0x100105cb
void __cdecl Sim_100105cb(int slot, int unused)
{
    int idx;
    (void)unused;
    if (slot >= 8)
        return;
    idx = Sim_10015c61(slot);
    if (idx == -1)
        return;
    if (S32(U32(PTRTAB(idx), 0x28), 0x4f0) != 2)
        return;
    SIM_PTR(u32a, 0x1012ff58)[slot] = 1;
    SIM_CALL_10016117(slot, 3);
}

// FUNCTION: HGSIM 0x10010700
void __cdecl Sim_10010700(int slot, int unused)
{
    int idx;
    (void)unused;
    if (slot >= 8)
        return;
    idx = Sim_10015c61(slot);
    if (idx == -1)
        return;
    if (S32(U32(PTRTAB(idx), 0x28), 0x4f0) != 2)
        return;
    SIM_PTR(u32a, 0x1012ff58)[slot] = 5;
    SIM_CALL_10016117(slot, 8);
}

// FUNCTION: HGSIM 0x10013eba
int __cdecl Sim_10013eba(void *obj, int unused)
{
    uint16_t count;
    uint32_t pair;
    (void)unused;
    if (U16(obj, 0x198) == 0)
        return 1;
    U16(obj, 0x198) = (uint16_t)(U16(obj, 0x198) - 1);
    count = U16(obj, 0x198);
    pair = U32(obj, 0x180 + 4u * count);
    /* the original pushes the low word with the top half of the object pointer; the callee reads 16 bits only */
    SIM_CALL_10015630(obj, (int16_t)(pair & 0xffff), (int16_t)(pair >> 16), 0);
    return 0;
}
