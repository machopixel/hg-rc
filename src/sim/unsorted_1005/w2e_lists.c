#include "w2e.h"
#include "../unsorted_1003/fns.h"
#include "../unsorted_1003/w2e.h"
#include "../unsorted_100d/fns.h"
#include "../unsorted_100f/w2e.h"
#include "../util/access.h"
#include "../util/util.h"

// FUNCTION: HGSIM 0x100530a9
void *__cdecl Sim_100530a9(const void *key)
{
    uint8_t *node = BYTES(G32(0x1015f3b0));
    uint8_t *result = 0;

    while (node != 0 && result == 0) {
        if (S16(node, 4) == -2 || S16(node, 4) == -1) {
            if (Str_Compare((const char *)node + 6, (const char *)BYTES(key) + 2) == 0)
                result = node + 4;
        } else if (S16(node, 4) == S16(key, 0)) {
            result = node + 4;
        }
        node = BYTES(U32(node, 0));
    }
    return result;
}

// FUNCTION: HGSIM 0x1005aa45
void __cdecl Sim_1005aa45(const char *a0, int a1, int a2, int a3, uint32_t a4, uint32_t a5,
                          const void *a6, uint32_t a7)
{
    int first, second, result;

    if (a3 < 0 || a3 >= 9)
        a3 = 4;
    U32(G32(0x1017d1f8), GS32(0x101c42a0) * 4u) = (uint32_t)Sim_100db770(a1);
    first = Sim_100db770(a1);
    first = Sim_100b3839((uint32_t)first);
    a2 = Sim_100db770(a2);
    second = Sim_100b3839((uint32_t)a2);
    result = Sim_1003aa7d(a0, U32(a6, 0x18), U32(a6, 0x1c), U32(a6, 0x20), (uint32_t)second,
                          (int)a7, first, (unsigned short)a5, 0);
    U32(G32(0x1017d1f4), GS32(0x101c42a0) * 4u) = (uint32_t)result;
    Sim_1003b50f(GS32(0x101c42a0), (uint16_t)a4);
    G32(0x101c42a0) = G32(0x101c42a0) + 1;
}
