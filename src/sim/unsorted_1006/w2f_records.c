#include "w2f.h"
#include "../util/access.h"

// FUNCTION: HGSIM 0x1006ab0c
void __cdecl Sim_1006ab0c(void *obj)
{
    uint32_t rec = U32(obj, 0x84);
    int pass;

    for (pass = 0; pass < 8; pass++) {
        if (S32(rec, 4) < 0) {
            rec += 0x128;
            continue;
        }
        if (S32(rec, 4) == 0x1c || S32(rec, 4) == 0x27 || S32(rec, 4) == 0x26) {
            U32(rec, 0) = 0;
            S32(rec, 8) = -1;
            U32(rec, 0x114) = 1;
            S32(rec, 0x20) = -2;
        }
    }
}
