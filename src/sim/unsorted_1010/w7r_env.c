#include <string.h>
#include "w7r.h"
#include "w6k.h"
#include "../util/access.h"

// FUNCTION: HGSIM 0x10100130
char *__cdecl Crt_GetEnv(const char *name)
{
    uint32_t p = G32(0x1017e8b0);
    uint32_t len;
    const char *e;

    if (p == 0) {
        if (G32(0x1017e8b8) == 0)
            return 0;
        if (W7R_FN_10107910() != 0)
            return 0;
        p = G32(0x1017e8b0);
        if (p == 0)
            return 0;
    }
    if (name == 0)
        return 0;
    len = (uint32_t)strlen(name);
    for (e = (const char *)(uintptr_t)U32(p, 0); e != 0; p += 4, e = (const char *)(uintptr_t)U32(p, 0)) {
        if ((uint32_t)strlen(e) > len && e[len] == '=' && Sim_101078d0(e, name, len) == 0)
            return (char *)e + len + 1;
    }
    return 0;
}

// FUNCTION: HGSIM 0x101009b0
int __cdecl Sim_101009b0(int code)
{
    uint32_t mode = G32(0x1017e8fc);

    if (mode == 1 || (mode == 0 && G32(0x1017e900) == 1))
        Sim_101083f0();
    W7R_FN_10108430(code);
    return ((W7rExitFn)(uintptr_t)G32(0x101596fc))(0xff);
}
