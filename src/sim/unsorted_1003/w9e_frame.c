#include "w9e.h"
#include "../unsorted_1000/w2i.h"
#include "../unsorted_100a/w8x.h"
#include "../unsorted_100a/w8y.h"
#include "../unsorted_100b/fns.h"
#include "../unsorted_100d/fns.h"
#include "../unsorted_100d/w5c.h"
#include "../unsorted_100f/w2e.h"
#include "../util/access.h"

#define CAMERA BYTES(G32(0x1014cae0))
#define DISPLAY16 G32(0x1017b940)

// FUNCTION: HGSIM 0x10034ec9
void __cdecl Sim_10034ec9(void)
{
    uint32_t saved;
    uint32_t p;
    int i;

    if (DISPLAY16 == 0 && G32(0x1015e53c) != 0) {
        Mem_Fill(BYTES(G32(0x109e3e20)), (int)G32(0x10138c70), G32(0x109e3ce8));
        Sim_100d32e5(GS32(0x1015e53c));
    }
    if (G32(0x1013a014) != 0) {
        W9E_CALL_100d2306(CAMERA);
        G32(0x1013a014) = 0;
    }
    if (DISPLAY16 == 0) {
        G32(0x109e3dc4) = U32(G32(0x109e3e00), 4) + 1;
        G32(0x109e3dc8) = U32(G32(0x109e3e00), 8) + 1;
        G32(0x109e3dcc) = U32(G32(0x109e3e00), 0);
        G32(0x109e3dd0) = G32(0x109e3e04);
        G32(0x109e3dd4) = G32(0x109e3e08);
        G32(0x109e3dd8) = G32(0x109e3e0c) + 1;
        G32(0x109e3ddc) = G32(0x109e3e10) + 1;
    }
    if (G32(0x1014cae8) != 0 && DISPLAY16 == 0) {
        W9E_CALL_100eb070(GBYTES(0x109e3e00), G32(0x10138c70));
        return;
    }
    Sim_100d255e(CAMERA);
    W9E_CALL_100d1f90(CAMERA);
    if (DISPLAY16 == 0) {
        if (G32(0x1014cb18) != 0 || G32(0x1014cb1c) != 0) {
            if (DISPLAY16 != 0) {
                /* Unreachable (the display mode was just tested): the original fills the 640x480 screen quad
                 * (four vertices of 0x24 bytes: x, y, +0x10 = 7, +0x18 = 0xff000000) through the driver's
                 * polygon routine. */
                uint32_t quad[4][9] = {{0}};

                quad[1][0] = 0x441fc000u; /* 639.0f */
                quad[2][0] = 0x441fc000u;
                quad[2][1] = 0x43ef8000u; /* 479.0f */
                quad[3][1] = 0x43ef8000u;
                for (i = 0; i < 4; i++) {
                    quad[i][4] = 7;
                    quad[i][6] = 0xff000000u;
                }
                (*SIM_PTR(W9eDriverPolyFn, 0x1014cb4c))(4, quad, G32(0x10138c70));
            } else {
                W9E_CALL_100eb070(GBYTES(0x109e3e00), G32(0x10138c70));
            }
        } else if (G32(0x1014cb04) != 0 || G32(0x1014cb08) != 0) {
            W9E_CALL_100a0def(CAMERA);
        }
    }
    if (G32(0x1015e540) != 0 && G32(0x1013a018) != 0 && GS32(0x1013a01c) != -1)
        Sim_100b53f0(GS32(0x1013a01c), (uint32_t *)(CAMERA + 0x1c), (uint32_t *)(CAMERA + 0x20),
                     (uint32_t *)(CAMERA + 0x24));
    G32(0x1015e54c) = 0;
    if (G32(0x1015e544) != 0) {
        saved = U32(CAMERA, 0x40);
        Sim_100d2545(CAMERA, 0x7f7fc99eu);
        G32(0x1014cb40) = W9E_ADDR(0x10001ab9);
        Sim_100a4ff0(BYTES(G32(0x1015e544)));
        G32(0x1015e54c) = G32(0x1015e54c) + G32(0x1017d138);
        Sim_100d2545(CAMERA, saved);
        G32(0x1014cb40) = W9E_ADDR(0x10001e42);
    }
    Sim_100a4d8e();
    G32(0x1015e54c) = G32(0x1015e54c) + G32(0x1017d138);
    if (G32(0x1015ef0c) != 0 && G32(0x1015e548) != 0 && G32(0x1013a008) != 0) {
        saved = U32(CAMERA, 0x3c);
        Sim_100d252c(CAMERA, 0x41000000u);
        G32(0x1014cb40) = W9E_ADDR(0x10002ce3);
        Sim_100a512a(BYTES(G32(0x1015e548)));
        G32(0x1015e54c) = G32(0x1015e54c) + G32(0x1017d138);
        Sim_100d252c(CAMERA, saved);
        G32(0x1014cb40) = W9E_ADDR(0x10001e42);
    }
    if (G32(0x1015e52c) != 0) {
        W9E_CALL_10087d83(CAMERA);
        for (i = 0; (p = SIM_PTR(u32a, 0x1036f2a0)[i]) != 0; i++)
            W9E_CALL_100357db(p);
    }
    Sim_100071f6();
    Sim_100d32e5(0);
}
