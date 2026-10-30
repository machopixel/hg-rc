#include "fns.h"
#include "../util/util.h"
#include "../util/access.h"
#include "../unsorted_1003/fns.h"

// FUNCTION: HGSIM 0x100c459f
int __cdecl Sim_100c459f(const unsigned char *entry, const unsigned char *other)
{
    int result = 1;

    if (Sim_1003356a(entry) != 0) {
        if ((uint32_t)Sim_10033580(entry) != U32(other, 4))
            result = 0;
    }
    return result;
}

// FUNCTION: HGSIM 0x100c48fa
void __cdecl Sim_100c48fa(void *obj, int mode)
{
    if (obj == 0 || U32(obj, 0x54) == 0)
        return;
    Sim_100c48c9(obj);
    if (mode == 0) {
        U8(U32(obj, 0x54), 0x58) = 0;
        U8(U32(obj, 0x54), 0x57) = 1;
    } else if (mode == 1) {
        U8(U32(obj, 0x54), 0x57) = 0;
        U8(U32(obj, 0x54), 0x58) = 1;
    } else {
        U8(U32(obj, 0x54), 0x57) = 0;
        U8(U32(obj, 0x54), 0x58) = 0;
    }
}
