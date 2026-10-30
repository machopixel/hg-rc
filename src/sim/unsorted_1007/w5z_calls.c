#include "w5z.h"
#include "../simmem.h"
#include "../util/access.h"
#include "fns.h"

typedef int(__cdecl *Fn100766ca)(int32_t key);
typedef void(__cdecl *Fn1002c8cb)(uint32_t a, uint32_t b, uint32_t c, uint32_t d);

// FUNCTION: HGSIM 0x10074c3d
int __cdecl Sim_10074c3d(int32_t key)
{
    int result = 0;
    int answer = SIM_FN(Fn100766ca, 0x100766ca)(key);

    if (answer == 0) {
        Sim_10074bfa(key);
        result = 1;
    } else if (answer == 1) {
        Sim_100751e4(key);
    }
    return result;
}

// FUNCTION: HGSIM 0x100788ba
void __cdecl Sim_100788ba(void *obj)
{
    if (S16(obj, 4) == 0 || U32(obj, 0x30) == 0)
        return;
    U32(obj, 0xf8) = G32(0x109e89d8);
    SIM_FN(Fn1002c8cb, 0x1002c8cb)(U32(obj, 0xec), 0, 0, 0);
}
