#include "w5x.h"
#include "fns.h"
#include "../unsorted_100f/w2e.h"
#include "../util/access.h"

// FUNCTION: HGSIM 0x100d1510
void *__cdecl Sim_100d1510(uint32_t first)
{
    void *node = SIM_CALL_100fef30(0x18);

    U32(node, 0) = first;
    U32(node, 0x14) = 0;
    U32(node, 0x10) = 0;
    U32(node, 0xc) = 0;
    U32(node, 8) = 0;
    U32(node, 4) = 0;
    return node;
}

// FUNCTION: HGSIM 0x100d15e0
void __cdecl Sim_100d15e0(void *node)
{
    List_Unlink(node);
    Sim_100d15fd(node);
}

// FUNCTION: HGSIM 0x100d15fd
void __cdecl Sim_100d15fd(void *node)
{
    if (node != 0) {
        Sim_100d15fd((void *)(uintptr_t)U32(node, 0xc));
        Sim_100d15fd((void *)(uintptr_t)U32(node, 8));
        if (U32(node, 0) != 0)
            SIM_CALL_100fefe0((void *)(uintptr_t)U32(node, 0));
        SIM_CALL_100fefe0(node);
    }
}

// FUNCTION: HGSIM 0x100d1dd0
void __cdecl Sim_100d1dd0(void)
{
    SIM_CALL_100fdc80(GBYTES(0x10152288), GBYTES(0x10152230));
    SIM_CALL_100fdc80(GBYTES(0x101523c0), GBYTES(0x10152290));
}

// FUNCTION: HGSIM 0x100d37ea
void __cdecl Sim_100d37ea(const char *name, int slot)
{
    Str_Copy((char *)GBYTES(0x10185620u + ((uint32_t)slot << 4)), name);
    SIM_CALL_100ba7d5(name, G32(0x1014ced4), 0);
    SIM_CALL_100bb2b0(name, G32(0x1014ced4));
}

// FUNCTION: HGSIM 0x100d3830
void __cdecl Sim_100d3830(uint32_t a, uint32_t b)
{
    SIM_CALL_100d3510(a, b, 0);
    G32(0x1017e654) = a;
    G32(0x10152424) = a;
}
