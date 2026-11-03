#include "w3p.h"
#include "../util/access.h"
#include "../util/w3p.h"

typedef void(__cdecl *Fn0)(void);
typedef void(__cdecl *Fn3)(uint32_t, uint32_t, uint32_t);

// FUNCTION: HGSIM 0x1003ccc4
void __cdecl Sim_1003ccc4(void)
{
    int i;

    for (i = 0; i < GS32(0x1015dc64); i++) {
        uint8_t *block = BYTES(U32(PTRTAB(i), 0x28));

        F32(block, 0x4ec) = (float)((double)F32(block, 0x4ec) / (double)GF32(0x101114fc));
    }
}
