#include "w2g.h"
#include "../simmem.h"
#include "../view/view.h"
#include "../unsorted_1008/w2g.h"

#define U8(p, off) (*(uint8_t *)((uint8_t *)(p) + (off)))
#define U16(p, off) (*(uint16_t *)((uint8_t *)(p) + (off)))
#define U32(p, off) (*(uint32_t *)((uint8_t *)(p) + (off)))
#define PTR(v) ((uint8_t *)(uintptr_t)(v))

// FUNCTION: HGSIM 0x100289a2
void __cdecl Sim_100289a2(void *dst, const void *src)
{
    U32(dst, 0xEC) = (uint32_t)(uintptr_t)src;
    U16(dst, 0xFC) = (uint16_t)U32(src, 4);
    U16(dst, 0xFE) = (uint16_t)U32(src, 8);
    U16(dst, 0x100) = (uint16_t)(U32(src, 0xC) - U32(src, 4) + 1);
    U16(dst, 0x102) = (uint16_t)(U32(src, 0x10) - U32(src, 8) + 1);
}

// FUNCTION: HGSIM 0x10028a84
void __cdecl Sim_10028a84(void *obj)
{
    U16(obj, 4) = 1;
}

// FUNCTION: HGSIM 0x1002a16b
void __cdecl Sim_1002a16b(int index)
{
    uint8_t *rec = SIM_PTR(uint8_t, 0x109E9BA0u + (uint32_t)index * 0x50u);

    U16(rec, 0) = 0;
    U16(rec, 6) = 0;
    U32(rec, 8) = 0xFFFFFFFFu;
    U32(rec, 0xC) = 0;
    U32(rec, 0x10) = 0;
    U8(rec, 0x18) = 0;
    U32(rec, 0x4C) = 0x80000000u;
}

// FUNCTION: HGSIM 0x1002a1c1
void __cdecl Sim_1002a1c1(void)
{
    int i;

    for (i = 0; i < 0xFE; i++)
        Sim_1002a16b(i);
}

// FUNCTION: HGSIM 0x1002b92d
void __cdecl Sim_1002b92d(void)
{
    int i;

    for (i = 0; i < 0x1E; i++) {
        int32_t *rect = SIM_PTR(int32_t, 0x109E8660u + (uint32_t)i * 0x14u);
        int32_t *point = SIM_PTR(int32_t, 0x109E8240u + (uint32_t)i * 8u);
        uint8_t *q;

        Sim_1008cf1f(rect, rect);
        View_ScaleRect(SIM_PTR(void, 0x109E3E20), rect, rect);
        Rect_ScalePoint(rect, point, point);
        q = PTR(SIM_PTR(uint32_t, 0x10138158)[i]);
        if (q != 0) {
            uint32_t *r;

            r = (uint32_t *)PTR(U32(PTR(U32(q, 4)), 4));
            r[0] = (uint32_t)(uintptr_t)SIM_PTR(void, 0x109E3E20);
            Rect_ScaleToRect(rect, (int32_t *)r, (int32_t *)r);
            r = (uint32_t *)PTR(U32(PTR(U32(q, 4)), 8));
            r[0] = (uint32_t)(uintptr_t)SIM_PTR(void, 0x109E3E20);
            Rect_ScaleToRect(rect, (int32_t *)r, (int32_t *)r);
            r = (uint32_t *)PTR(U32(PTR(U32(q, 4)), 0xC));
            r[0] = (uint32_t)(uintptr_t)SIM_PTR(void, 0x109E3E20);
        }
    }
}
