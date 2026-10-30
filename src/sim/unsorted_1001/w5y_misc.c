#include "w5y.h"
#include "w2e.h"
#include "w2j.h"
#include "w4u.h"
#include "../unsorted_1000/w2g.h"
#include "../unsorted_1000/w2l.h"
#include "../unsorted_1002/w5y.h"
#include "../util/access.h"

// FUNCTION: HGSIM 0x10010a73
void __cdecl Sim_10010a73(void)
{
    int mode = Sim_100109e7(GS32(0x1015d100));

    if (mode >= 0)
        SIM_FN(void(__cdecl *)(int, int), 0x10010564)(mode, 0);
}

// FUNCTION: HGSIM 0x10010b38
void __cdecl Sim_10010b38(void)
{
    int mode = Sim_100109e7(GS32(0x1015d100));

    if (mode >= 0)
        SIM_FN(void(__cdecl *)(int, int), 0x10010632)(mode, 0);
}

// FUNCTION: HGSIM 0x1001403a
int __cdecl Sim_1001403a(void *obj, short value)
{
    S32(obj, 0xf0) = value;
    return SIM_FN(int(__cdecl *)(void *), 0x100cb4fb)(obj) > 0;
}

// FUNCTION: HGSIM 0x100144bb
float __cdecl Sim_100144bb(uint32_t unused, float f, int n)
{
    float x = (float)((double)f / (double)3840.0f);

    (void)unused;
    return SIM_FN(float(__cdecl *)(float, int, int), 0x1001655f)(x, (int)((uint32_t)n << 1), 1);
}

// FUNCTION: HGSIM 0x1001f415
int __cdecl Sim_1001f415(int a)
{
    SIM_FN(int(__cdecl *)(int, int), 0x10039aca)(0xb, a);
    SIM_FN(int(__cdecl *)(void), 0x1009e1ef)();
    Sim_1000df50();
    Sim_1001f6c0();
    G32(0x109f4228) = G32(0x1015dc78) + 0x389;
    return 1;
}

// FUNCTION: HGSIM 0x1001f4ba
void __cdecl Sim_1001f4ba(void)
{
    Sim_1000ede4();
    Sim_1002001f();
    Sim_1001f335();
}
