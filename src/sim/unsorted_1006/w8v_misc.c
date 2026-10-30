#include "w8v.h"
#include "../util/access.h"
#include "../unsorted_1000/w2i.h"
#include "../unsorted_1003/w2b.h"
#include "../unsorted_1004/fns.h"
#include "../unsorted_1004/w2j.h"
#include "../unsorted_100b/w6g.h"
#include "../unsorted_100d/fns.h"
#include "../unsorted_100d/w5c.h"

#define CAMERA BYTES(G32(0x1014cae0))

/* Sets the camera up again after its state was copied in (Sim_100449fc): the three camera steps. */
static void CameraSteps(void)
{
    W8V_CALL_100d2306(CAMERA);
    Sim_100d255e(CAMERA);
    W8V_CALL_100d1f90(CAMERA);
}

/*
 * Renders with a temporary camera: saves the camera state (Sim_10044992) and a few globals, applies the preset,
 * sets camera +0x18 to value and copies view (whose +0x18 becomes 1) into the camera, renders the target
 * (0x100a512a) or the whole list at 0x1036f2a0, optionally calls 0x10062b7c(extra), then restores everything.
 */
// FUNCTION: HGSIM 0x100619a0
void __cdecl Sim_100619a0(int preset, uint32_t value, uint8_t *view, uint32_t target, uint32_t extra)
{
    uint32_t saved[7];
    uint32_t keep9fc, keep18, keep650, p;
    int32_t i;

    keep9fc = G32(0x1014c9fc);
    G32(0x1014c9fc) = 0;
    keep18 = U32(CAMERA, 0x18);
    keep650 = G32(0x1017e650);
    Sim_10044992((const uint32_t *)CAMERA, saved);
    Sim_100d32e5(preset);
    U32(CAMERA, 0x18) = value;
    U32(view, 0x18) = 1;
    Sim_100449fc(CAMERA, view);
    CameraSteps();
    if ((G32(0x1014cb04) != 0 || G32(0x1014cb08) != 0) && G32(0x1014cb1c) == 0)
        W8V_CALL_100a0def(CAMERA);
    if (target != 0) {
        W8V_CALL_100a512a(target);
    } else {
        W8V_CALL_10087d83(CAMERA);
        for (i = 0; (p = SIM_PTR(u32a, 0x1036f2a0)[i]) != 0; i++)
            W8V_CALL_100a4f03(p);
    }
    Sim_100071f6();
    if (extra != 0)
        W8V_CALL_10062b7c(extra, 0, 0, 0);
    G32(0x1017e650) = keep650;
    Sim_10035469();
    U32(CAMERA, 0x18) = keep18;
    G32(0x1013a014) = 1;
    Sim_100449fc(CAMERA, saved);
    CameraSteps();
    G32(0x1013a014) = 0;
    G32(0x1014c9fc) = keep9fc;
}

/* Reloads the resource named at 0x10185620 + 16 * slot (slot 0x11 when `second`, else 0x10) and refreshes it. */
// FUNCTION: HGSIM 0x10061bc5
void __cdecl Sim_10061bc5(int second)
{
    int32_t slot = second != 0 ? 0x11 : 0x10;

    if (W8V_CALL6_100ba7d5(SIM_PTR(char, 0x10185620 + (uint32_t)slot * 16), (char *)(uintptr_t)G32(0x1014ced4), 0) == 0)
        return;
    Sim_100bb2b0(SIM_PTR(const uint32_t, 0x10185620 + (uint32_t)slot * 16), (const char *)(uintptr_t)G32(0x1014ced4));
    W8V_CALL_100d3427(slot);
}
