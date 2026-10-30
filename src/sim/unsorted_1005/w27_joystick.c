#include "w27.h"

// FUNCTION: HGSIM 0x1005621b
uint32_t __cdecl Sim_1005621b(uint32_t id, void *dev)
{
    (void)id;
    (void)dev;
#ifdef W27J_MUTATE /* the checker's negative check only */
    return 0;
#else
    return 1;
#endif
}

// FUNCTION: HGSIM 0x100567cb
uint32_t __cdecl Sim_100567cb(void *dev)
{
    (void)dev;
#ifdef W27J_MUTATE /* the checker's negative check only */
    return 0;
#else
    return 1;
#endif
}
