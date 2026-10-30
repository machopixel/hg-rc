#include "w5z.h"
#include "../simmem.h"
#include "../util/access.h"

typedef uint32_t(__cdecl *Fn100fd850)(const char *name, const char *mode);
typedef uint32_t(__cdecl *Fn100febc0)(void *buf, uint32_t size, uint32_t count, uint32_t handle);
typedef void(__cdecl *Fn1004ee50)(void *obj);

// FUNCTION: HGSIM 0x1004db2e
int __cdecl Sim_1004db2e(const char *name)
{
    G32(0x1015f344) = SIM_FN(Fn100fd850, 0x100fd850)(name, SIM_PTR(const char, 0x1013b9a0));
    return G32(0x1015f344) == 0;
}

// FUNCTION: HGSIM 0x1004db5b
int __cdecl Sim_1004db5b(void *buf, uint32_t count)
{
    uint32_t got = SIM_FN(Fn100febc0, 0x100febc0)(buf, 1, count, G32(0x1015f344));

    return got < count ? 2 : 0;
}

// FUNCTION: HGSIM 0x1004f158
void __cdecl Sim_1004f158(void *obj)
{
    uint32_t block = U32(obj, 0x28);

    U32(obj, 0x94) = 0;
    U32(obj, 0x90) = U32(block, 0x5ff);
    if (G32(0x1015f3a8) == 0)
        SIM_FN(Fn1004ee50, 0x1004ee50)(obj);
}
