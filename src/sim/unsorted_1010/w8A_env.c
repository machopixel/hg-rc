#include <string.h>
#include "w8A.h"
#include "w7s.h"
#include "w7r.h"
#include "w6k.h"
#include "w2k.h"
#include "../unsorted_100f/w6i.h"
#include "../util/access.h"

#define ADDR(p) ((uint32_t)(uintptr_t)(p))
#define ENVIRON G32(0x1017e8b0)    /* the environment copy (pointer array) */
#define ENVIRON0 G32(0x1017e8b4)   /* the array the copy started from */
#define WENVIRON G32(0x1017e8b8)   /* the 16-bit environment list */

typedef int32_t(__stdcall *W8aSetEnvironmentVariableAFn)(const char *name, const char *value);

// FUNCTION: HGSIM 0x1010b2a0
int32_t __cdecl Crt_SetEnv(char *option, int32_t primary)
{
    char *eq;
    int32_t remove, ix;
    uint32_t namelen;
    u32a *env;

    if (option == 0)
        return -1;
    eq = Crt_MbsChr(option, '=');
    if (eq == 0 || eq == option)
        return -1;
    remove = eq[1] == 0;
    env = (u32a *)(uintptr_t)ENVIRON;
    if (ADDR(env) == ENVIRON0) {
        env = (u32a *)(uintptr_t)ADDR(Sim_1010b530((char **)env));
        ENVIRON = ADDR(env);
    }
    if (env == 0) {
        if (primary != 0 && WENVIRON != 0) {
            if (Sim_10107910() != 0)
                return -1;
        } else {
            u32a *p;

            if (remove)
                return 0;
            p = Heap_Malloc(4);
            ENVIRON = ADDR(p);
            if (p == 0)
                return -1;
            *p = 0;
            if (WENVIRON == 0) {
                p = Heap_Malloc(4);
                WENVIRON = ADDR(p);
                if (p == 0)
                    return -1;
                *p = 0;
            }
        }
        env = (u32a *)(uintptr_t)ENVIRON;
    }
    namelen = (uint32_t)(eq - option);
    ix = Sim_1010b4b0(option, namelen);
    if (ix >= 0 && env[0] != 0) {
        if (remove) {
            u32a *p = &env[ix];
            void *grown;

            Heap_FreeBlock(*p);
            while (*p != 0) {
                *p = p[1];
                ix++;
                p++;
            }
            grown = Heap_Realloc((void *)env, (uint32_t)ix * 4u);
            if (grown != 0)
                ENVIRON = ADDR(grown);
        } else {
            env[ix] = ADDR(option);
        }
    } else {
        u32a *grown;

        if (remove)
            return 0;
        if (ix < 0)
            ix = -ix;
        grown = Heap_Realloc((void *)env, (uint32_t)ix * 4u + 8u);
        if (grown == 0)
            return -1;
        grown[ix] = ADDR(option);
        grown[ix + 1] = 0;
        ENVIRON = ADDR(grown);
    }
    if (primary != 0) {
        char *buf = Heap_Malloc((uint32_t)strlen(option) + 2u);

        if (buf == 0)
            return 0;
        memcpy(buf, option, strlen(option) + 1);
        buf[namelen] = 0;
        SIM_API(W8aSetEnvironmentVariableAFn, 0x10ac5610)(buf, remove ? 0 : buf + namelen + 1);
        Heap_FreeBlock(ADDR(buf));
    }
    return 0;
}

// FUNCTION: HGSIM 0x101073d0
void __cdecl Crt_Abort(void)
{
    Sim_10108430(10);
    W8A_FN_10107670(0x16);
    Proc_ExitQuick(3);
}
