#include "w5z.h"
#include "../simmem.h"
#include "../util/access.h"
#include "../unsorted_1000/w2j.h"
#include "../unsorted_1001/w2j.h"
#include "../unsorted_1003/w2e.h"
#include "../unsorted_1005/w3s.h"

typedef uint32_t(__cdecl *Fn1005264d)(uint32_t a, uint32_t b);
typedef int(__cdecl *Fn10016117)(uint32_t a, uint32_t b);
typedef int(__cdecl *Fn10010699)(int a, int b);
typedef uint32_t(__cdecl *Fn10016d83)(uint32_t a, uint32_t b);

// FUNCTION: HGSIM 0x100100a2
uint32_t __cdecl Sim_100100a2(uint32_t a, uint32_t unused)
{
    (void)unused;
    G32(0x1012ff58) = 0;
    return SIM_FN(Fn1005264d, 0x1005264d)(G32(0x1013bb40), a);
}

// FUNCTION: HGSIM 0x10010acd
int __cdecl Sim_10010acd(void)
{
    int result = Sim_1000fe60(4);

    if (result != 0)
        result = SIM_FN(Fn10016117, 0x10016117)(4, 2);
    return result;
}

// FUNCTION: HGSIM 0x10010b0b
int __cdecl Sim_10010b0b(void)
{
    int mode = Sim_100109e7((int)G32(0x1015d100));

    if (mode >= 0)
        mode = SIM_FN(Fn10010699, 0x10010699)(mode, 0);
    return mode;
}

// FUNCTION: HGSIM 0x10010f70
void __cdecl Sim_10010f70(void)
{
    int i;

    if (G32(0x10a3ade8) != 0)
        Mono_Clear();
    for (i = 0; i < GS32(0x104737e0); i++) {
        Sim_10052ad6(i);
        Sim_1001661f((uint32_t)i);
    }
}

// FUNCTION: HGSIM 0x1001661f
uint32_t __cdecl Sim_1001661f(uint32_t a)
{
    return SIM_FN(Fn10016d83, 0x10016d83)(a, 0);
}
