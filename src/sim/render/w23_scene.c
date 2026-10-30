#include "w23.h"
#include "w22.h"
#include "../util/access.h"

#define W23S_CALL_10098f8c SIM_FN(void(__cdecl *)(int32_t, int32_t, void *, int32_t, int32_t), 0x10098f8c)

// FUNCTION: HGSIM 0x1009b772
void __cdecl Sim_1009b772(int32_t count, void *verts, uint32_t flags)
{
    if (G32(0x1014c4c8 + G32(0x1015d1ac) * 4u) == 0)
        return;
    switch (flags & 0xf000) {
    case 0x3000:
        W23S_CALL_10098f8c((int32_t)(flags & 0xff0) >> 4, count, verts, -1, 1);
        return;
    case 0:
        Sim_1009ef50(count, verts, 0xf0);
        Sim_1009ef50(count, verts, flags | 0xa000);
        return;
    case 0x8000:
    case 0x9000:
#ifdef W23S_MUTATE /* the checker's negative check only */
    case 0xb000:
#endif
        return;
    default:
        Sim_1009ef50(count, verts, flags);
        return;
    }
}
