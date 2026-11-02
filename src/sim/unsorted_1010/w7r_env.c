#include <string.h>
#include "w7r.h"
#include "w6k.h"
#include "../util/access.h"
#include "../crt/hg_rest.h"

// FUNCTION: HGSIM 0x10100130
char *__cdecl Crt_GetEnv(const char *name)
{
    return hg_getenv(name);     /* C runtime replacement, the rest: standard C's getenv (crt/hg_rest.h) */
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
