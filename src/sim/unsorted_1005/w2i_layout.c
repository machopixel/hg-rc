#include "w2i.h"
#include "fns.h"
#include "../view/view.h"
#include "../util/access.h"

#define AREA(obj) ((const int32_t *)(uintptr_t)U32(obj, 0xec))
#define POINT(obj, off) ((int32_t *)(BYTES(obj) + (off)))

/* Scales the point stored at obj + off in place. */
static void ScaleAt(void *obj, unsigned off)
{
    Rect_ScalePoint(AREA(obj), POINT(obj, off), POINT(obj, off));
}

// FUNCTION: HGSIM 0x10050671
void __cdecl Sim_10050671(void *obj)
{
    int i;

    U32(obj, 0xb4) = 0xa3d;
    U32(obj, 0xb8) = 0xccd;
    ScaleAt(obj, 0xb4);
    U32(obj, 0xbc) = 0x51f;
    U32(obj, 0xc4) = 0xfae1;
    U32(obj, 0xcc) = 0xfae1;
    ScaleAt(obj, 0xbc);
    ScaleAt(obj, 0xc4);
    ScaleAt(obj, 0xcc);
    U32(obj, 0xc0) = G32(0x109E8780);
    U32(obj, 0xc8) = G32(0x109E8788);
    U32(obj, 0xd0) = G32(0x109E879C);
    G32(0x10151D68) = U32(AREA(obj), 4) + U32(obj, 0xbc);
    G32(0x10151D6C) = U32(obj, 0xc0);
    G32(0x10151D80) = U32(AREA(obj), 4) + U32(obj, 0xc4);
    G32(0x10151D84) = U32(obj, 0xc0);
    G32(0x10151D98) = U32(AREA(obj), 4) + U32(obj, 0xc4);
    G32(0x10151D9C) = U32(obj, 0xc8);
    G32(0x10151DB0) = U32(AREA(obj), 4) + U32(obj, 0xbc);
    G32(0x10151DB4) = U32(obj, 0xc8);
    for (i = 0; i < 0x100; i++) {
        uint32_t low = (i >= 0xe0 && i < 0x100) ? ((uint32_t)i & 0x1f) : ((uint32_t)i & 0xf);

        if ((int32_t)low <= 0xf)
            U8(GBYTES(0x1015E3C0u), i) = (uint8_t)((uint32_t)i - low);
        else
            U8(GBYTES(0x1015E3C0u), i) = (uint8_t)((uint32_t)i - 0xf);
    }
}

// FUNCTION: HGSIM 0x10050c5b
void __cdecl Sim_10050c5b(void)
{
    static const uint32_t order[7] = { 0x109E8928, 0x109E88E8, 0x109E890C, 0x109E8938, 0x109E894C, 0x109E8950, 0x109E8954 };
    int i;

    Sim_100505f0((void *)(uintptr_t)G32(0x109E8940));
    Sim_10050671((void *)(uintptr_t)G32(0x109E8918));
    for (i = 0; i < 7; i++)
        Sim_10050a63((void *)(uintptr_t)G32(order[i]));
    for (i = 0; i < 7; i++)
        Sim_100508df((void *)(uintptr_t)G32(order[i]));
    for (i = 0; i < 7; i++)
        Sim_1005095b((void *)(uintptr_t)G32(order[i]));
    for (i = 0; i < 7; i++)
        Sim_100509df((void *)(uintptr_t)G32(order[i]));
    Sim_10050b66((void *)(uintptr_t)G32(0x109E8938));
    Sim_10050b66((void *)(uintptr_t)G32(0x109E88E8));
    Sim_10050b66((void *)(uintptr_t)G32(0x109E890C));
    Sim_10050b66((void *)(uintptr_t)G32(0x109E8928));
    Sim_10050b66((void *)(uintptr_t)G32(0x109E894C));
    Sim_10050b66((void *)(uintptr_t)G32(0x109E8950));
    Sim_10050b66((void *)(uintptr_t)G32(0x109E8954));
}

// FUNCTION: HGSIM 0x10057900
int __cdecl Sim_10057900(void)
{
    return 1;
}
