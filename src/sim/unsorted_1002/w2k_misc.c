#include "w2k.h"
#include "../util/access.h"

// FUNCTION: HGSIM 0x10027acf
const char *__cdecl Sim_10027acf(uint32_t code)
{
    int i;

    for (i = 0; i < 0x61; i++) {
        if (G32(0x101349E8 + i * 4) == code)
            break;
    }
    if (i < 0x61)
        return (const char *)(uintptr_t)G32(0x101348B8 + i * 4);
    return (const char *)GBYTES(0x10134FE4);
}

// FUNCTION: HGSIM 0x10028965
void __cdecl Sim_10028965(void *obj, uint32_t value)
{
    U32(obj, 8) = value;
}

// FUNCTION: HGSIM 0x10028a15
void __cdecl Sim_10028a15(void *obj, uint32_t value)
{
    U32(obj, 0xf0) = value;
}

// FUNCTION: HGSIM 0x1002dc46
int __cdecl Sim_1002dc46(const void *obj)
{
    return G32(0x10138BC8u + U32(obj, 0x34) * 12u) != 0;
}
