#include "w2h.h"

// FUNCTION: HGSIM 0x10083490
void __cdecl Sim_10083490(void)
{
}

// FUNCTION: HGSIM 0x1008d405
void __cdecl Sim_1008d405(const void *obj)
{
    const volatile int32_t *p = (const volatile int32_t *)obj;

    /* Only the reads are kept, so that a bad pointer faults as before. */
    (void)p[1];
    (void)p[2];
    (void)p[3];
    (void)p[4];
}
