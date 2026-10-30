#include "w2b.h"
#include "../simmem.h"

#define U32(p, off) (*(uint32_t *)((unsigned char *)(p) + (off)))

// FUNCTION: HGSIM 0x10042908
int __cdecl Sim_10042908(const unsigned char *id)
{
    uint32_t index = id[0];
    uint32_t kind = (uint32_t)(uint16_t)((uint32_t)id[1] << 8);
    unsigned char *obj;

    if (kind == 0x200 && *SIM_PTR(int32_t, 0x1015EC58) != 0) {
        int slot = Sim_1003dbc0(index);
        unsigned char *root = (unsigned char *)(uintptr_t)*SIM_PTR(uint32_t, 0x1015D004);
        uint16_t *keys = (uint16_t *)(uintptr_t)U32(root, 0x16);

        if (keys[slot] != 0xFA01) {
            obj = (unsigned char *)(uintptr_t)SIM_PTR(uint32_t, 0x109E8B40)[index];
            if (U32(obj, 0x160) != 0)
                return 1;
            if (Sim_10079e3b(id) == 0)
                return 0;
            if (*(unsigned char *)(uintptr_t)U32(obj, 0x168) > 0)
                return 0;
            return U32(obj, 0x164) == 0;
        }
    }
    return 1;
}
