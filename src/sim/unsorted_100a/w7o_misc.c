#include "w7o.h"
#include "w3o.h"
#include "w5a.h"
#include "../util/access.h"
#include "../render/w2m.h"
#include "../render/w33.h"
#include "../render/w3r.h"
#include "../view/view.h"
#include "../view/w2a.h"
#include "../view/w2l.h"
#include "../unsorted_1007/w2e.h"
#include "../unsorted_1008/w2g.h"
#include "../unsorted_100d/fns.h"
#include "../unsorted_100d/w5c.h"
#include "../unsorted_100f/w2e.h"
#include "../unsorted_100f/w6h.h"
#include "../unsorted_100f/w6i.h"
#include "../unsorted_1010/w6k.h"

#define RECORD(index) (BYTES(SIM_PTR(uint8_t, 0x101c65e0)) + (uint32_t)(index) * 0x14au)
#define VIEW SIM_PTR(void, 0x109e3e20)

// FUNCTION: HGSIM 0x100a7c4c
void __cdecl Sim_100a7c4c(int32_t index)
{
    uint32_t i;

    Sim_100a78eb(U32(RECORD(index), 6));
    U32(RECORD(index), 6) = 0;
    for (i = 0; i < 0x20; i++) {
        if (U32(RECORD(index), 0x2a + i * 8) != 0) {
            Sim_100a78eb(U32(RECORD(index), 0x2a + i * 8));
            U32(RECORD(index), 0x2a + i * 8) = 0;
        }
        if (U32(RECORD(index), 0x2e + i * 8) != 0) {
            Sim_100a78eb(U32(RECORD(index), 0x2e + i * 8));
            U32(RECORD(index), 0x2e + i * 8) = 0;
        }
    }
    U16(RECORD(index), 4) = 0;
    Crt_StrNSet((char *)RECORD(index) + 0x12a, 0, 0x20);
    W7O_CALL_101049c0(U32(RECORD(index), 0));
}

// FUNCTION: HGSIM 0x100a7e55
int32_t __cdecl Sim_100a7e55(int32_t index)
{
    uint32_t header = U32(RECORD(index), 6);
    uint32_t count = U16(header, 0xc);
    uint32_t table = header + 0xe;
    uint32_t i;

    for (i = 0; i < count; i++) {
        uint32_t entry = table + i * 0x18;
        uint32_t size;
        int buffer;

        if (U32(entry, 4) == 0)
            continue;
        size = U32(entry, 8);
        buffer = Sim_100a78d9((int)size);
        if (buffer == 0)
            return -1;
        W7O_CALL_10105fe0(U32(RECORD(index), 0), U32(entry, 4), 0);
        if ((uint32_t)File_ReadChunked(U32(RECORD(index), 0), (uint32_t)buffer, size) != size) {
            Sim_100a78eb((uint32_t)buffer);
            return -1;
        }
        U32(RECORD(index), 0x2a + i * 8) = (uint32_t)buffer;
    }
    return 0;
}

#define DEFERRED(i) (U32(G32(0x1017d168), (uint32_t)(i) * 8))
/* the renderer's state call [0x109e3d40], 1.0's numbering */
#define W7O_STATE RENDER_SLOT(int(__cdecl *)(unsigned, unsigned), 0x109e3d40)

// FUNCTION: HGSIM 0x100a4c45
void __cdecl Sim_100a4c45(void)
{
    uint8_t saved[0x68];
    int32_t i;

    if (G32(0x1017d134) != 0)
        for (i = 0; i < GS32(0x1017d134); i++)
            W7O_CALL_100a539e(DEFERRED(i));
    if (G32(0x101c9ea0) != 0)
        for (i = GS32(0x1017d134); i < (int32_t)(G32(0x1017d134) + G32(0x101c9ea0) - G32(0x1017d130)); i++)
            W7O_CALL_100a539e(DEFERRED(i));
    if (G32(0x1017d130) != 0) {
        /* 1.2 (0x10091065): in its 3DFX / PowerVR mode, around the last group, state 0xe (1.0's 0xf) off and on */
#ifdef W7OV_MUTATE /* the checker's negative check only: the state call left out */
        if (0)
#else
        if (g_w33Voodoo != 0 && G32(0x1014cb1c) != 1)
#endif
            W7O_STATE(0xf, 0);
        Sim_100787f4(saved);
        G32(0x1014caec) = 0;
        G32(0x1014cb24) = 0;
        G32(0x1014cb1c) = 4;
        G32(0x1014cb20) = 0;
        for (i = (int32_t)(G32(0x1017d134) + G32(0x101c9ea0) - G32(0x1017d130));
             i < (int32_t)(G32(0x1017d134) + G32(0x101c9ea0)); i++)
            W7O_CALL_100a539e(DEFERRED(i));
        Mem_Move(SIM_PTR(void, 0x1014cae8), saved, 0x68);
        if (g_w33Voodoo != 0 && G32(0x1014cb1c) != 1)
            W7O_STATE(0xf, 1);
    }
    G32(0x1017d130) = 0;
    G32(0x101c9ea0) = 0;
    G32(0x1017d134) = 0;
}

// FUNCTION: HGSIM 0x100ab920
void __cdecl Sim_100ab920(void)
{
    int32_t point[2];
    int i;

    for (i = 0; i < 8; i++) {
        int32_t *rect = SIM_PTR(int32_t, 0x10185400 + (uint32_t)i * 0x14);

        Sim_1008cf1f(rect, rect);
        View_ScaleRect(VIEW, rect, rect);
    }
    Sim_1008d07b((const int32_t *)(uintptr_t)G32(0x1014bfe4), (int32_t *)(uintptr_t)G32(0x1014bfe4));
    View_ScalePoint(VIEW, (const int32_t *)(uintptr_t)G32(0x1014bfe4), (int32_t *)(uintptr_t)G32(0x1014bfe4));
    Sim_100d32e5(0);
    W7O_CALL_100d2306((void *)(uintptr_t)G32(0x1014cae0));
    Sim_100d255e((void *)(uintptr_t)G32(0x1014cae0));
    G32(0x1013a014) = 0;
    point[0] = GS32(0x1014cb50);
    point[1] = 0;
    Sim_1008d07b(point, point);
    View_ScalePoint(VIEW, point, point);
    G32(0x1014cb50) = (uint32_t)point[0];
    W7O_CALL_1007dd90();
    Sim_1000856d();
}

// FUNCTION: HGSIM 0x100ae2e0
uint32_t *__cdecl File_OpenRead(const char *name)
{
    uint32_t *handle = Heap_Malloc(0xc);

    if (handle == 0)
        return 0;
    handle[0] = (uint32_t)(uintptr_t)Sim_100fd850(name, SIM_PTR(const char, 0x1014d1e4));     /* "rb" */
    if (handle[0] == 0) {
        Heap_FreeBlock((uint32_t)(uintptr_t)handle);
        return 0;
    }
    handle[1] = 0;
    handle[2] = 0;
    return handle;
}
