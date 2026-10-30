#include "w4w.h"
#include "w2i.h"
#include "../win/w4w_api.h"
#include "../util/access.h"

typedef void(__cdecl *W4wReport)(const void *format, uint32_t value);

// FUNCTION: HGSIM 0x10083692
int __cdecl Sim_10083692(void)
{
    G32(0x1017b7d8) = 0;
    if (G32(0x1017b3c0) != 0) {
        if (G32(0x1017b3c4) != 0) {
            SIM_API(W4wSelectObjectFn, W4W_IAT_SELECTOBJECT)((void *)(uintptr_t)G32(0x1017b3c0),
                                                            (void *)(uintptr_t)G32(0x103729dc));
            SIM_API(W4wBoolPtrFn, W4W_IAT_DELETEOBJECT)((void *)(uintptr_t)G32(0x1017b3c4));
        }
        if (G32(0x1015e9a0) != 0)
            SIM_API(W4wHeapFreeFn, W4W_IAT_HEAPFREE)(W4W_GAME_HEAP, 1, BYTES(G32(0x1015e9a0)));
        SIM_API(W4wReleaseDCFn, W4W_IAT_RELEASEDC)((void *)(uintptr_t)G32(0x1017b8f0), (void *)(uintptr_t)G32(0x1017b3c0));
    }
    G32(0x1017b3c0) = 0;
    G32(0x1017b3c4) = 0;
    G32(0x1017b7c8) = 0;
    G32(0x1017b7cc) = 0;
    G32(0x1017b7d0) = 0;
    G32(0x1015e9a0) = 0;
    U32(G32(0x1015e698), 0) = G32(0x1015e9a0);
    G32(0x1017b7d4) = 0;
    return 0;
}

// FUNCTION: HGSIM 0x10083d5b
int __cdecl Sim_10083d5b(int x1, int y1, int x2, int y2)
{
    int top = y1 - (int)SIM_API(W4wU32IntFn, W4W_IAT_GETSYSTEMMETRICS)(0xf);

    G32(0x103729c4) = (uint32_t)SIM_API(W4wSetDIBitsFn, W4W_IAT_SETDIBITSTODEVICE)(
        (void *)(uintptr_t)G32(0x1017b3c0), x1, top, (uint32_t)(x2 - x1 + 1), (uint32_t)(y2 - y1 + 1), x1, y1, 0,
        G32(0x109e3d18), BYTES(G32(0x1015e9a0)), GBYTES(0x109e38c0), 1);
    if (G32(0x103729c4) == 0)
        ((W4wReport)(void *)Sim_10083495)(GBYTES(0x1014afd0), SIM_API(W4wGetLastErrorFn, W4W_IAT_GETLASTERROR)());
    return G32(0x103729c4) != G32(0x109e3d18) ? -1 : 0;
}

// FUNCTION: HGSIM 0x1008c6b6
int __cdecl Sim_1008c6b6(void *record)
{
    if (U32(record, 4) != 0) {
        SIM_API(W4wHeapFreeFn, W4W_IAT_HEAPFREE)(W4W_GAME_HEAP, 1, BYTES(U32(record, 4)));
        U32(record, 4) = 0;
    }
    return ((int(__cdecl *)(uint32_t))(uintptr_t)U32(G32(0x1015e688), 0x18))(U32(record, 0));
}
