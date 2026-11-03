#include "w9d.h"
#include "w8x.h"
#include "../render/w2n.h"
#include "../unsorted_100b/w6g.h"
#include "../unsorted_100f/w2e.h"
#include "../util/access.h"

// FUNCTION: HGSIM 0x1009a137
void __cdecl Sim_1009a137(void *obj)
{
    uint32_t pane = U32(obj, 0);
    char label[12];

    /* the original loads the current unit's record pointer and never uses it */
    (void)*(volatile u32a *)(SIM_PTR(u32a, 0x109e8b40) + GS32(0x1017b8c8));
    Sim_1009aac9(obj);
    if (G32(0x1015d1ac) != 4) {
        float fx = (float)((int32_t)(U32(pane, 0xc) - U32(pane, 4) + 1) >> 1);
        float fy = (float)((int32_t)(U32(pane, 0x10) - U32(pane, 8) + 1) >> 1);
        const char *text = (const char *)(uintptr_t)U32(U32(obj, 0x70), 0);
        uint32_t len = Str_Length(text);
        void *shape;

        Str_CopyN(label, text, 8);
        if ((int32_t)len >= 8)
            len = 7;
        label[len + 1] = 0;
        shape = W9D_RADAR_CALL_100ba7d5(label, (void *)(uintptr_t)G32(0x1014cec0), 0);
        if (shape != 0) {
            uint32_t x = (uint32_t)Render_Ftol((double)fx);
            uint32_t y = (uint32_t)Render_Ftol((double)fy);

            W9D_CALL_100f0205((void *)(uintptr_t)pane, shape, 0, x, y);
            Sim_100bb2b0((const uint32_t *)label, (const char *)(uintptr_t)G32(0x1014cec0));
            Sim_1009a3b3(obj);
        }
    }
    Sim_1009a6f3(obj);
}
