#include "w28.h"
#include "../../platform/plat.h"
#include "../util/access.h"
#include "../unsorted_1003/w2h.h"
#include "../unsorted_100f/w2e.h"
#include "../unsorted_100f/w6i.h"

typedef int32_t(__cdecl *W28DescFn)(char *kind, int32_t *id, void *extra);
typedef int32_t(__cdecl *W28CheckFn)(const char *arg);

void(__cdecl *g_w28Trace)(const char *format, ...);

/* the renderer's entry points: where each goes and its name in the DLL's strings */
static const uint32_t kW28Entries[][2] = {
    {0x109e3d24u, 0x1013a10cu},
    {0x109e3d28u, 0x1013a11cu},
    {0x109e3d2cu, 0x1013a128u},
    {0x109e3d30u, 0x1013a134u},
    {0x109e3d34u, 0x1013a13cu},
    {0x109e3d3cu, 0x1013a14cu},
    {0x109e3d40u, 0x1013a15cu},
    {0x109e3d44u, 0x1013a168u},
    {0x109e3d48u, 0x1013a17cu},
    {0x109e3d4cu, 0x1013a188u},
    {0x109e3d50u, 0x1013a198u},
    {0x109e3d54u, 0x1013a1a8u},
    {0x109e3d58u, 0x1013a1b8u},
    {0x109e3d5cu, 0x1013a1c8u},
    {0x109e3d60u, 0x1013a1d8u},
    {0x109e3d64u, 0x1013a1ecu}};

// FUNCTION: HGSIM 0x1003650d
void __cdecl Sim_1003650d(const char *path)
{
    void *lib;
    W28DescFn desc;
    W28CheckFn check;
    char kind[0x100];
    int32_t id, r;
    uint32_t i;

    lib = Plat_LoadLibrary(path);
    if (lib == 0) {
        if (g_w28Trace != 0)
            g_w28Trace("LoadPlugin(%s): the library does not load\n", path);
        return;
    }
    desc = (W28DescFn)Plat_LibraryFunction(lib, (const char *)GBYTES(0x1013a0ec));
    if (desc == 0)
        return;
    r = desc(kind, &id, 0);
    if (Str_CompareNoCase(kind, (const char *)GBYTES(0x1013a0f8)) != 0)
        return;
    if (G32(0x1015e578) != 0) {
        if (*(const char *)GBYTES(0x1015e57c) == 0) {
            if ((int32_t)G32(0x1015e5bc) != id) {
                Plat_FreeLibrary(lib);
                return;
            }
        } else if (Str_CompareNoCase((const char *)GBYTES(0x1015e57c), path) != 0) {
            Plat_FreeLibrary(lib);
            return;
        }
    }
    check = (W28CheckFn)Plat_LibraryFunction(lib, (const char *)GBYTES(0x1013a100));
    if (*(const char *)GBYTES(0x1015e59c) != 0)
        r = check((const char *)GBYTES(0x1015e59c));
    else
        r = check(0);
    if (g_w28Trace != 0)
        g_w28Trace("LoadPlugin(%s): kind=%s id=%d CheckFunc=%d; game wants id=%d name='%s'\n", path, kind, id, r,
                   (int32_t)G32(0x1015e5bc), (const char *)GBYTES(0x1015e57c));
    if (Sim_10036820((uint32_t)r, id) == 0)
        return;
    G32(0x109e3d20) = (uint32_t)id;
    for (i = 0; i < sizeof(kW28Entries) / sizeof(kW28Entries[0]); i++)
        G32(kW28Entries[i][0]) = (uint32_t)(uintptr_t)Plat_LibraryFunction(lib, (const char *)GBYTES(kW28Entries[i][1]));
#ifdef W28P_MUTATE /* the checker's negative check only */
    G32(0x1015e5bc) = (uint32_t)id + 1;
#else
    G32(0x1015e5bc) = (uint32_t)id;
#endif
    Str_Copy((char *)GBYTES(0x1015e57c), path);
    if (G32(0x1015e570) != 0)
        Plat_FreeLibrary((void *)(uintptr_t)G32(0x1015e568));
    G32(0x1015e570) = 1;
    G32(0x1015e568) = (uint32_t)(uintptr_t)lib;
}

// FUNCTION: HGSIM 0x1003695b
void __cdecl Sim_1003695b(const char *kind, const char *name, uint32_t index)
{
    if (g_w28Trace != 0)
        g_w28Trace("PluginChoice kind=%s name=%s index=%ld\n", kind, name != 0 ? name : "(null)", (long)index);
    if (Str_CompareNoCase(kind, (const char *)GBYTES(0x1013a214)) == 0) {
        G32(0x1015e578) = 1;
        if (name == 0)
            G32(0x1015e5bc) = index;
        else
            Str_Copy((char *)GBYTES(0x1015e57c), name);
    } else if (Str_CompareNoCase(kind, (const char *)GBYTES(0x1013a21c)) == 0) {
        G32(0x1015e5c0) = 1;
        if (name == 0)
            G32(0x1015e604) = index;
        else
            Str_Copy((char *)GBYTES(0x1015e5c4), name);
    } else if (Str_CompareNoCase(kind, (const char *)GBYTES(0x1013a224)) == 0) {
        G32(0x1015e578) = 1;
        Str_Copy((char *)GBYTES(0x1015e59c), name);
    }
}
