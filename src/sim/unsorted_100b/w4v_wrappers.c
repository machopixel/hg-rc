#include "w4v.h"
#include "../util/access.h"

#define RECORD(i) (0x101a2100u + (uint32_t)(i) * 0x88u)
#define LIST(i) ((uint32_t *)GBYTES(RECORD(i) + 0x84u))

// FUNCTION: HGSIM 0x100b41dc
void *__cdecl Sim_100b41dc(int index, TimerCallback callback, uint32_t interval, uint32_t arg)
{
    return Sim_100cc6ea(LIST(index), callback, interval, arg);
}

// FUNCTION: HGSIM 0x100b4213
void __cdecl Sim_100b4213(void)
{
    int i;

    for (i = 0; i < GS32(0x1017d218); i++) {
        if ((G32(RECORD(i) + 0x18u) & 0x800) == 0 && G32(RECORD(i) + 0x84u) != 0)
            Sim_100cc8c5(LIST(i));
    }
}

// FUNCTION: HGSIM 0x100b427b
void __cdecl Sim_100b427b(int index, void *node)
{
    Sim_100cc79a(LIST(index), node);
}

// FUNCTION: HGSIM 0x100b42aa
void __cdecl Sim_100b42aa(int index)
{
    Sim_100cc830(LIST(index));
}
