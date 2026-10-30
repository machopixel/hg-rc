#include "w5a.h"
#include "w2e.h"
#include "../util/util.h"
#include "../util/access.h"
#include "../win/w4u.h"

/* The three state changes below are the same except for the stored value and the code. */
static void SetState(int index, uint32_t state, uint32_t code)
{
    int slot;
    uint32_t sub;

    if (index >= 8)
        return;
    slot = Sim_10015c61(index);
    if (slot == -1)
        return;
    sub = U32(PTRTAB(slot), 0x28);
    if (S32(sub, 0x4f0) != 2)
        return;
    SIM_PTR(u32a, 0x1012ff58)[index] = state;
    W5A_CALL_10016117(index, code);
}

// FUNCTION: HGSIM 0x1001042f
void __cdecl Sim_1001042f(int index, int unused)
{
    SetState(index, 2, 0xd);
}

// FUNCTION: HGSIM 0x10010564
void __cdecl Sim_10010564(int index, int unused)
{
    SetState(index, 2, 2);
}

// FUNCTION: HGSIM 0x10010699
void __cdecl Sim_10010699(int index, int unused)
{
    SetState(index, 4, 7);
}

// FUNCTION: HGSIM 0x10012a72
int __cdecl Sim_10012a72(void *obj, int16_t key)
{
    int16_t first, second;

    if (((int)key & 0xf000) == 0)
        return (uint16_t)key;
    W5A_CALL_100135d4(obj, (uint16_t)key, 0xfffffffdu, &first, &second);
    if (S16(obj, 0x1a6) == 2 && U32(obj, 0x1dc) != 0)
        return (uint16_t)first;
    return second;
}

// FUNCTION: HGSIM 0x10015a63
void __cdecl Sim_10015a63(void *obj)
{
    uint32_t a, b, c;
    int index;
    uint8_t *entry;

    Sim_1001570f(obj);
    if (W5A_CALL_100529a2(U32(obj, 4), &a, &c, &b) == 0)
        return;
    index = Sim_100ca350(U32(obj, 4), a, b, c);
    if (index == -1)
        return;
    entry = GBYTES(0x109a8648) + (uint32_t)index * 0x54u;
    U16(entry, 0x1c) |= 1;
    U32(entry, 0) = U32(obj, 4) | 0x200;
    U16(obj, 0x1a8) = (uint16_t)((uint32_t)index | 0x100);
}

// FUNCTION: HGSIM 0x1001f3ac
void __cdecl Sim_1001f3ac(void)
{
    G32(0x1015d1c4) += G32(0x1015dc80);
    if (GS32(0x1015d1c4) < 0x16a)
        return;
    G32(0x1015d1c4) = 0;
    if (G32(0x1015d1bc) != 0)
        return;
    if ((G32(0x101321c0) & 8) == 0)
        return;
    if (W5A_CALL_1009e5d0() == 0)
        return;
    if (GS32(0x101321a8) == -1)
        return;
    Cd_PlayTrack(GS32(0x101321a8));
    Cd_UpdateStatus();
}
