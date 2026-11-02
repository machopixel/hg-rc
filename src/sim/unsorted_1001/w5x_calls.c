#include "w5x.h"
#include "w2j.h"
#include "w4u.h"
#include "../unsorted_1000/w2j.h"
#include "../view/view.h"
#include "../win/w4u.h"
#include "../util/access.h"
#include "../crt/hg_winheap.h"

// FUNCTION: HGSIM 0x10010aa0
void __cdecl Sim_10010aa0(void)
{
    int v = Sim_100109e7((int)G32(0x1015d100));

    if (v >= 0)
        SIM_CALL_10010700(v, 0);
}

// FUNCTION: HGSIM 0x10010aec
void __cdecl Sim_10010aec(void)
{
    if (Sim_1000fe60(4) != 0)
        SIM_CALL_10016117(4, 5);
}

// FUNCTION: HGSIM 0x10010b65
void __cdecl Sim_10010b65(void)
{
    int v = Sim_100109e7((int)G32(0x1015d100));

    if (v >= 0)
        SIM_CALL_100105cb(v, 0);
}

// FUNCTION: HGSIM 0x100125a3
void __cdecl Sim_100125a3(void *p)
{
    /* the original also tests for 5 and 2 first, which cannot change the outcome */
    if (U32(p, 0) == 8)
        SIM_CALL_1001274b(p);
}

// FUNCTION: HGSIM 0x10013dbd
void __cdecl Sim_10013dbd(void *obj, int16_t value)
{
    if (Sim_10079feb((int32_t)U32(obj, 0)) != 0)
        SIM_CALL_10017b3e(obj, 2000.0f);
    U8(U32(obj, 0x54), 0x26) = 0;
    U8(U32(obj, 0x54), 0x28) = 0;
    SIM_CALL_100be0f0(obj, value);
}

// FUNCTION: HGSIM 0x1001f454
void __cdecl Sim_1001f454(void)
{
    SIM_CALL_10039c00();
    if (GS32(0x1015dc78) >= GS32(0x109f4228))
        G32(0x109f4228) = G32(0x1015dc78) + 0x389;
}

// FUNCTION: HGSIM 0x1001f47d
void __cdecl Sim_1001f47d(void)
{
    Sim_1001f399();
    /* (Cd_Shutdown here: nothing without a CD; no CD audio, DECISIONS.md "dropped features") */
    SIM_CALL_1003a052();
    SIM_CALL_1004724c(GBYTES(0x10132220), G32(0x1015d1b8));
    Hg_HeapFree(G32(0x1017b8f8), 1, (void *)(uintptr_t)G32(0x1015d1b8));
}
