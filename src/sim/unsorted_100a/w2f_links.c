#include "w2f.h"
#include "../util/access.h"

// FUNCTION: HGSIM 0x100aa7cd
void *__cdecl Sim_100aa7cd(void *p)
{
    while (U32(p, 0) != 0)
        p = (void *)(uintptr_t)U32(p, 0);
    return p;
}

// FUNCTION: HGSIM 0x100aa7e7
uint32_t __cdecl Sim_100aa7e7(const void *p)
{
    return U32(p, 0);
}

// FUNCTION: HGSIM 0x100aa7fc
uint32_t __cdecl Sim_100aa7fc(const void *p)
{
    return U32(p, 8);
}
