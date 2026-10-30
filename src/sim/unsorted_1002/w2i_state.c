#include "w2i.h"
#include "../util/access.h"

#define SELECTED GS32(0x10134890)
#define ENTRY(i) (0x1096fcc0u + (uint32_t)(i) * 0x156u)

// FUNCTION: HGSIM 0x100258d7
void *__cdecl Sim_100258d7(void)
{
    if (SELECTED != -1 && U32(GBYTES(ENTRY(SELECTED)), 0x20) != 0)
        return GBYTES(0x1015DBD8);
    SELECTED = -1;
    return 0;
}

// FUNCTION: HGSIM 0x1002590c
int __cdecl Sim_1002590c(void)
{
    if (GS32(0x1013488C) > 0 && U32(GBYTES(ENTRY(GS32(0x1013488C))), 8) != 0
        && U32(GBYTES(ENTRY(GS32(0x1013488C))), 0x10) == G32(0x1017B8C8)) {
        SELECTED = GS32(0x1013488C);
        U32(GBYTES(ENTRY(GS32(0x1013488C))), 0x20) = 1;
        return 1;
    }
    SELECTED = -1;
    return 0;
}
