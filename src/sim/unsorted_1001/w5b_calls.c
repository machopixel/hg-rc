#include "w5b.h"
#include "../util/util.h"
#include "../util/access.h"
#include "w2h.h"
#include "../unsorted_1004/fns.h"
#include "../unsorted_1001/w2l.h"
#include "../unsorted_1000/w2j.h"
#include "../unsorted_1004/w3p.h"
#include "../view/view.h"

/* The common body of the three first functions. */
static void SetMode(int value, uint32_t number, int argument)
{
    int index;
    uint32_t block;

    if (value >= 8)
        return;
    index = Sim_10015c61(value);
    if (index == -1)
        return;
    block = U32(PTRTAB(index), 0x28);
    if (S32(block, 0x4f0) != 2)
        return;
    G32(0x1012ff58u + (uint32_t)value * 4u) = number;
    SIM_CALL_10016117(value, argument);
}

// FUNCTION: HGSIM 0x100104fd
void __cdecl Sim_100104fd(int value, int unused)
{
    (void)unused;
    SetMode(value, 2, 0xf);
}

// FUNCTION: HGSIM 0x10010632
void __cdecl Sim_10010632(int value, int unused)
{
    (void)unused;
    SetMode(value, 3, 5);
}

// FUNCTION: HGSIM 0x10010767
void __cdecl Sim_10010767(int value, int unused)
{
    (void)unused;
    SetMode(value, 6, 0xb);
}

// FUNCTION: HGSIM 0x10010951
void __cdecl Sim_10010951(int mode)
{
    int first = Sim_1000fe60(1);
    int second = Sim_1000fe60(2);

    if (mode == 0) {
        if (first != 0 && second == 0)
            mode = 1;
        else if (first == 0 && second != 0)
            mode = 2;
        else if (first == 0 && second == 0)
            return;
    } else {
        if (mode == 1 && first == 0)
            return;
        if (mode == 2 && second == 0)
            return;
    }
    G32(0x1015d100) = (uint32_t)mode;
    SIM_CALL_100200d1(mode, 0);
}

// FUNCTION: HGSIM 0x1001393e
int __cdecl Sim_1001393e(void *obj, uint16_t value, uint16_t unused)
{
    uint32_t last;
    uint16_t result = value;

    (void)unused;
    if (Sim_100152fc(U32(obj, 8), value, 2) == 0) {
        result = 0;
        last = 0;
    } else {
        last = (uint32_t)Sim_10052aaf((int)U32(obj, 8));
        if ((int32_t)last == GS32(0x1017b8c8))
            last = (uint32_t)SIM_CALL_10020256(4, (int)U32(obj, 0x10));
    }
    return (int)((last & 0xffff0000u) | result);
}

// FUNCTION: HGSIM 0x10013999
void __cdecl Sim_10013999(void *obj, int16_t unused)
{
    (void)unused;
    U32(U32(obj, 0x54), 8) = 0;
    U32(obj, 0xd4) = 0;
    if (Sim_10079feb((int32_t)U32(obj, 0)) != 0) {
        if (S16(obj, 0x1a6) != 0)
            Sim_10017aa9(obj);
        else
            SIM_CALL_10017b3e(obj, 4000.0f);
    }
}

// FUNCTION: HGSIM 0x10016d83
void __cdecl Sim_10016d83(int kind, const uint16_t *words)
{
    if (G32(0x1015ec58) != 0)
        Sim_100409be(kind, words);
    if (words != 0 && (int16_t)words[0] != 0) {
        if ((int16_t)words[0] == 7)
            SIM_CALL_10016de7(kind, words[1]);
    } else {
        SIM_CALL_10016632(kind);
    }
}
