#include "w9c.h"
#include "../unsorted_1002/w5c.h"
#include "../unsorted_1003/fns.h"
#include "../unsorted_1003/w9c.h"
#include "../unsorted_100a/w2i.h"
#include "../unsorted_100a/w5y.h"
#include "../unsorted_100a/w5z.h"
#include "../unsorted_100a/w6g.h"
#include "../unsorted_100f/w2e.h"
#include "../unsorted_100f/w6h.h"
#include "../util/access.h"
#include "../util/util.h"

#define W9C_NAME(obj) ((const char *)(uintptr_t)Sim_100a9b7f((void *)(uintptr_t)(obj)))

// FUNCTION: HGSIM 0x1009d90e
uint32_t __cdecl Sim_1009d90e(void *node, int32_t keep, int32_t index)
{
    uint32_t created = 0;
    uint32_t obj;
    uint32_t owner = 0;

    if (node == 0)
        return 0;
    obj = U32(node, 8);
    if (obj == 0)
        return 0;
    if (keep == 0 && Crt_StrStr(W9C_NAME(obj), SIM_PTR(const char, 0x1014c9a0)) == 0
        && Crt_StrStr(W9C_NAME(obj), SIM_PTR(const char, 0x1014c9a4)) == 0) {
        Sim_10029ab3(obj, (uint32_t)(uintptr_t)SIM_FN(void *, 0x100020e5));
        Sim_1003b986(obj);
    } else {
        uint32_t fresh;
        uint8_t *slot;
        int k;

        if (index >= 0 && index < 0x19)
            owner = U32(PTRTAB(index), 0x28);
        fresh = (uint32_t)(uintptr_t)Sim_100a9830((void *)(uintptr_t)U32(node, 0xc), 10);
        if (fresh == 0)
            return 0;
        Sim_100a9b8d((void *)(uintptr_t)fresh, (const char *)(uintptr_t)U32(obj, 0x90));
        slot = GBYTES(0x109b9e20) + (uint32_t)GS32(0x1015ea80) * 0x7eu;
        G32(0x1015ea80) += 1;
        Mem_Move(slot, GBYTES(0x109b9e20) + (uint32_t)Sim_1003b485(obj) * 0x7eu, 0x7e);
        U32(slot, 0x60) = fresh;
        U32(slot, 0) = (uint32_t)index;
        U32(slot, 4) = 0xffffffffu;
        U32(slot, 0x48) = (uint32_t)Sim_1003b485(U32(node, 0xc));
        for (k = 0; k < 4; k++)
            U32(slot, 0x50 + 4 * k) = 0;
        if (Crt_StrStr(W9C_NAME(obj), SIM_PTR(const char, 0x1014c9a8)) != 0)
            U32(owner, 0x498 + 4 * U32(owner, 0x5fb)) = fresh;
        if (U32(node, 0) != 0)
            U32(U32(node, 0), 0xc) = fresh;
        obj = fresh;
        created = 1;
    }
    Sim_100a9d47((void *)(uintptr_t)obj, U32(node, 0x10), U32(node, 0x14), U32(node, 0x18));
    Sim_100aa0da((void *)(uintptr_t)obj, U32(node, 0x1c), U32(node, 0x20), U32(node, 0x24), 0);
    U32(obj, 4) = Sim_1009d90e((void *)(uintptr_t)U32(node, 0), (int32_t)created, index);
    U32(obj, 8) = Sim_1009d90e((void *)(uintptr_t)U32(node, 4), keep, index);
    U32(obj, 0) = U32(node, 0xc);
    return obj;
}
