#include "fns.h"
#include "../util/util.h"
#include "../util/access.h"
#include "../view/view.h"

// FUNCTION: HGSIM 0x10008c51
void __cdecl Sim_10008c51(void *obj)
{
    Sim_10008c69(obj, U32(obj, 0x2c));
}

// FUNCTION: HGSIM 0x10008c77
void __cdecl Sim_10008c77(void *obj)
{
    uint32_t link = U32(obj, 0x30);

    if (link == 0 || U32(link, 0xc) == 0)
        Sim_10008c69(obj, U32(obj, 0x2c));
    else
        Sim_10008c69(obj, U32(link, 0xc));
}

// FUNCTION: HGSIM 0x1000c172
void __cdecl Sim_1000c172(void *obj)
{
    uint32_t record = U32(obj, 0x28);

    /* The original also loads the 0x28 field of that table entry and never uses it; the read is kept so a bad entry faults as before. */
    (void)*(volatile uint32_t *)(BYTES(PTRTAB(GS32(0x1017b8c8))) + 0x28);
    if (Sim_1009c929(100) < S32(obj, 0x1e8))
        U16(record, 0x560) |= 0x80;
    if (S32(obj, 0) == 8 || S32(obj, 0) == 5)
        U16(record, 0x560) |= 0x80;
}

// FUNCTION: HGSIM 0x1000c680
int32_t __cdecl Sim_1000c680(const void *obj)
{
    uint32_t flags;

    if (obj == 0)
        return -1;
    flags = U32(obj, 0xf0);
    if ((flags & 0x200) != 0 && Sim_10079feb(S32(PTRTAB(flags & 0xff), 0)) != 0)
        return 1;
    return 0;
}

// FUNCTION: HGSIM 0x1000edce
uint32_t __cdecl Sim_1000edce(void)
{
    return SIM_PTR(u32a, 0x1012df30)[Sim_1009c929(10)];
}
