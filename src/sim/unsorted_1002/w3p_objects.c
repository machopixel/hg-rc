#include "w3p.h"
#include "../util/access.h"
#include "../util/util.h"

typedef void(__cdecl *ObjectFn)(void *object, int value);

// FUNCTION: HGSIM 0x1002c6e2
void __cdecl Sim_1002c6e2(const void *p, int flag)
{
    int chance;
    int i;

    if (U32(U32(p, 0), 4) != G32(0x1017b8c8))
        return;
    chance = flag != 0 ? 5 : 2;
    for (i = 0; i < 0x1e; i++) {
        if (Sim_1009c929(10) < chance) {
            uint8_t *object = BYTES(SIM_PTR(u32a, 0x109e88e0)[i]);

            (*(ObjectFn *)(object + 0x12c))(object, (int)S16(object, 6) + 1);
        }
    }
}
