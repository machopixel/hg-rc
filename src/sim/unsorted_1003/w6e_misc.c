#include <string.h>

#include "w6e.h"
#include "../unsorted_100d/w5b.h"
#include "../unsorted_100d/w5z.h"
#include "../unsorted_100f/w2e.h"
#include "../util/util.h"
#include "../util/access.h"

typedef void(__cdecl *W6eSlot4Fn)(uint32_t, uint32_t, uint32_t, uint32_t);
typedef void(__cdecl *W6eSlot0Fn)(void);
typedef uint32_t(__cdecl *W6eDescFn)(char *kind, uint32_t *id, uint32_t extra);
typedef uint32_t(__cdecl *W6eNumDeviceFn)(uint32_t arg);

// FUNCTION: HGSIM 0x10035478
void __cdecl Sim_10035478(void)
{
    W6E_CALL_10007890();
    Sim_100d3491();
    Sim_100d33ec();
    if (G32(0x109e3dac) != 0) {
        ((W6eSlot4Fn)(uintptr_t)U32(G32(0x1015e68c), 0x20))(G32(0x109e3e04) + 1, G32(0x109e3e08) + 1,
                                                            G32(0x109e3e0c), G32(0x109e3e10));
        memcpy(SIM_PTR(void, 0x109e3e00), SIM_PTR(void, 0x1015e518), 0x14);
        G32(0x10144fa0) = G32(0x1014bef4);
        G32(0x109e3dac) = 0;
    } else if (G32(0x1017b920) != 0) {
        ((W6eSlot0Fn)(uintptr_t)U32(G32(0x1015e68c), 0x18))();
    }
}

// FUNCTION: HGSIM 0x10036027
void __cdecl Plugin_ProbeRaster(const char *path, uint32_t arg)
{
    char kind[0x100];
    uint32_t id;
    void *module;
    W6eDescFn desc;
    W6eNumDeviceFn count;

    if (G32(0x1015e56c) != 0)
        return;
    module = SIM_API(W6eLoadLibraryFn, 0x10ac55e4)(path);
    if (module == 0)
        return;
    desc = (W6eDescFn)SIM_API(W6eGetProcAddressFn, 0x10ac55dc)(module, SIM_PTR(char, 0x1013a078));
    if (desc != 0) {
        desc(kind, &id, 0);
        if (W6E_CALL_100ff1e0(kind, SIM_PTR(char, 0x1013a084)) == 0) {
            count = (W6eNumDeviceFn)SIM_API(W6eGetProcAddressFn, 0x10ac55dc)(module, SIM_PTR(char, 0x1013a08c));
            if (count != 0)
                G32(0x1015e56c) = count(arg);
            if (G32(0x1015e56c) != 0)
                Str_Copy(SIM_PTR(char, 0x1015e57c), path);
        }
    }
    SIM_API(W6eFreeLibraryFn, 0x10ac55d8)(module);
}

// FUNCTION: HGSIM 0x10038eea
int __cdecl Sim_10038eea(void *obj, int id)
{
    union {
        uint32_t u;
        float f;
    } x, y, z;
    uint32_t node;

    Sim_100a9cfb((void *)(uintptr_t)U32(U32(obj, 0), 0x48), &x.u, &y.u, &z.u);
    node = G32(0x109e3760);
    if (id == -1) {
        for (; node != 0; node = U32(node, 0x690)) {
            float dx = x.f - W6E_F(node, 0x680);
            float dy = y.f - W6E_F(node, 0x684);
            float dz = z.f - W6E_F(node, 0x688);

            /* the original tests only "not greater or equal": a NaN distance counts as near */
            if (!(W6E_CALL_10084b8c(dx, dy, dz) >= 1000.0f))
                return 1;
        }
    } else {
        for (; node != 0; node = U32(node, 0x690)) {
            if (S32(node, 0x124) == id)
                return 1;
        }
    }
    return 0;
}
