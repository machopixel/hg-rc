#include "w13.h"
#include "../simmem.h"
#include "../util/access.h"
#include "../util/util.h"
#include "../util/w3o.h"
#include "../unsorted_1002/w2i.h"
#include "../unsorted_1004/fns.h"
#include "../unsorted_1004/w6e.h"
#include "../unsorted_100a/w5b.h"
#include "../unsorted_1006/w8v.h"
#include "../unsorted_1007/w2e.h"
#include "../unsorted_1007/w5x.h"
#include "../unsorted_1007/w5z.h"
#include "../unsorted_1008/w7o.h"
#include "../unsorted_1009/w8x.h"
#include "../unsorted_100a/w5x.h"
#include "../unsorted_100b/w12.h"
#include "../unsorted_100c/w10.h"
#include "../unsorted_100c/w8z.h"
#include "../unsorted_100f/w2e.h"

/* The HUD library's frame fill (still original). */
#define W13_CALL_100eb070 SIM_FN(void(__cdecl *)(uint32_t, uint32_t), 0x100eb070)

/* The player's object (first dword of its record), read again at every use as the original does. */
#define W13_OBJ(rec) U32((rec), 0)

/* Modes Sim_100788ba does not take over. */
static int W13_OwnMode(int32_t mode)
{
    return mode == 1 || mode == 9 || mode == 10 || mode == 0xc || mode == 0xb || mode == 0xd || mode == 2;
}

/* Renders the camera copy view around the player's node and frees the node again. */
static void W13_Render(uint8_t *pane, uint32_t rec, uint32_t *view, uint32_t extra)
{
    Sim_100aa351(BYTES(U32(W13_OBJ(rec), 0x48)));
    Sim_100619a0(S32(pane, 0xc), 0x40000000u, (uint8_t *)view, 0, extra);
    Sim_100aa39f(BYTES(U32(W13_OBJ(rec), 0x48)));
}

// FUNCTION: HGSIM 0x100782a9
void __cdecl Sim_100782a9(uint8_t *pane)
{
    uint32_t view[7]; /* camera copy: position, angles, flag (filled by Sim_10044992) */
    uint8_t saved[0x68];
    uint32_t rec;
    void *sel;

    if (S16(pane, 4) == 0 || S32(pane, 0x30) == 0)
        return;
    U32(pane, 0xf8) = G32(0x109e89d8);
    if (S16(pane, 6) == 1 && !W13_OwnMode(S32(pane, 0x30))) {
        if (G32(0x10179d24) != 0) {
            if (Sim_1009c929(10) < 7)
                G32(0x10179d24) = 0;
            Sim_100788ba(pane);
            return;
        }
        if (Sim_1009c929(10) < 3)
            G32(0x10179d24) = 1;
    } else if (S16(pane, 6) > 2 && !W13_OwnMode(S32(pane, 0x30))) {
        Sim_100788ba(pane);
        return;
    }
    rec = U32(PTRTAB(G32(0x1017b8c8)), 0x28);
    switch (U32(pane, 0x30)) {
    case 0:
    case 2:
    case 10:
    case 11:
    case 12:
    case 13:
        break;
    case 3:
        Sim_10044992((const uint32_t *)(uintptr_t)G32(0x1014cae0), view);
        Sim_100787f4(saved);
        if (G32(0x1096fbd0) != 0)
            FL(view, 0xc) = (float)((double)FL(W13_OBJ(rec), 0x74) + (double)FL(W13_OBJ(rec), 0x68));
        else
            FL(view, 0xc) = (float)((double)FL(W13_OBJ(rec), 0x68) + (double)GFL(0x1012b804));
        view[4] = 0;
        W13_Render(pane, rec, view, 0);
        if (G32(0x1096fbd0) != 0)
            Sim_1008d4f1((int32_t *)(uintptr_t)U32(pane, 0xec), 0x14);
        else
            Sim_100788fc(pane, 0x14, (uint32_t)(uintptr_t)SIM_PTR(char, 0x10149b14)); /* "VP_RER" */
        Mem_Move(SIM_PTR(uint8_t, 0x1014cae8), saved, 0x68);
        break;
    case 4:
        Sim_10044992((const uint32_t *)(uintptr_t)G32(0x1014cae0), view);
        Sim_10078857(saved);
        Sim_10044b2c(&view[4], &view[3], &view[5], &view[0], &view[1], &view[2]);
        W13_Render(pane, rec, view, rec);
        Sim_1008d4f1((int32_t *)(uintptr_t)U32(pane, 0xec), 0x14);
        Mem_Move(SIM_PTR(uint8_t, 0x1014cae8), saved, 0x68);
        break;
    case 5:
        Sim_10044992((const uint32_t *)(uintptr_t)G32(0x1014cae0), view);
        Sim_100787f4(saved);
        view[0] = U32(W13_OBJ(rec), 0x58);
        view[1] = U32(W13_OBJ(rec), 0x5c);
        view[2] = U32(W13_OBJ(rec), 0x60);
        view[4] = 0x42b40000u; /* 90.0f */
        view[5] = 0;
        W13_Render(pane, rec, view, 0);
        Sim_100788fc(pane, 0x14, (uint32_t)(uintptr_t)SIM_PTR(char, 0x10149b0c)); /* "VP_DWN" */
        Mem_Move(SIM_PTR(uint8_t, 0x1014cae8), saved, 0x68);
        break;
    case 6:
        if (U8(G32(0x1017b8c4), 9) != 0) {
            Sim_10099ef3(BYTES(U32(pane, 0xec)));
            Sim_1008d4f1((int32_t *)(uintptr_t)U32(pane, 0xec), 0x14);
        }
        break;
    case 7:
        sel = Sim_100258d7();
        if (sel == 0) {
            Sim_1002590c();
            sel = Sim_100258d7();
        }
        if (sel == 0) {
            W13_CALL_100eb070(U32(pane, 0xec), 0);
        } else {
            Sim_100787f4(saved);
            U32(sel, 0x10) = 0;
            Sim_100619a0(S32(pane, 0xc), 0x40000000u, (uint8_t *)sel, 0, 0);
            Mem_Move(SIM_PTR(uint8_t, 0x1014cae8), saved, 0x68);
        }
        Sim_100788fc(pane, 0x14, (uint32_t)(uintptr_t)SIM_PTR(char, 0x10149b04)); /* "VP_WPN" */
        break;
    case 8:
        Sim_100c7deb(pane);
        break;
    case 9:
        Sim_100cf96c(pane);
        break;
    default:
        Sim_100b66a0();
        break;
    }
}
