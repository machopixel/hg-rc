#include "fns.h"
#include "../simmem.h"
#include "../view/view.h"

/* Field of a structure at a byte offset. */
#define FLD(type, p, off) (*(type *)((uint8_t *)(p) + (off)))
#define AREA(obj) ((const int32_t *)(uintptr_t)FLD(uint32_t, obj, 0xec))
#define POINT(obj, off) ((int32_t *)((uint8_t *)(obj) + (off)))

/* Scales the point stored at obj + off in place. */
static void ScaleAt(void *obj, unsigned off)
{
    Rect_ScalePoint(AREA(obj), POINT(obj, off), POINT(obj, off));
}

/* Half of a value, rounded toward zero. */
static int32_t Half(int32_t v)
{
    return v / 2;
}

// FUNCTION: HGSIM 0x100505f0
void __cdecl Sim_100505f0(void *obj)
{
    uint32_t index = *SIM_PTR(uint32_t, 0x1017b8c8);
    uint8_t *entry = (uint8_t *)(uintptr_t)*SIM_PTR(uint32_t, 0x109e8b40 + index * 4u);
    uint8_t *other = (uint8_t *)(uintptr_t)FLD(uint32_t, entry, 0x28);

    if (FLD(int32_t, other, 0x613) == 0) {
        FLD(int32_t, obj, 0xac) = 0xcccd;
        FLD(int32_t, obj, 0xb0) = 0xf5c;
    } else {
        FLD(int32_t, obj, 0xac) = 0x11ec;
        FLD(int32_t, obj, 0xb0) = 0xa3d;
    }
    ScaleAt(obj, 0xac);
}

// FUNCTION: HGSIM 0x100508df
void __cdecl Sim_100508df(void *obj)
{
    FLD(int32_t, obj, 0x7c) = 0;
    FLD(int32_t, obj, 0x80) = 0x8000;
    FLD(int32_t, obj, 0x84) = 0x8000;
    FLD(int32_t, obj, 0x88) = 0x2db7;
    ScaleAt(obj, 0x84);
    ScaleAt(obj, 0x7c);
}

// FUNCTION: HGSIM 0x1005095b
void __cdecl Sim_1005095b(void *obj)
{
    FLD(int32_t, obj, 0x8c) = 0;
    FLD(int32_t, obj, 0x90) = 0x3333;
    FLD(int32_t, obj, 0x94) = 0xea4e;
    FLD(int32_t, obj, 0x98) = 0x5555;
    ScaleAt(obj, 0x94);
    ScaleAt(obj, 0x8c);
}

// FUNCTION: HGSIM 0x100509df
void __cdecl Sim_100509df(void *obj)
{
    FLD(int32_t, obj, 0x9c) = 0;
    FLD(int32_t, obj, 0xa0) = 0x8000;
    FLD(int32_t, obj, 0xa4) = 0x8000;
    FLD(int32_t, obj, 0xa8) = 0x2db7;
    ScaleAt(obj, 0xa4);
    ScaleAt(obj, 0x9c);
}

// FUNCTION: HGSIM 0x10050a63
void __cdecl Sim_10050a63(void *obj)
{
    uint32_t w, h, x;

    FLD(int32_t, obj, 0x34) = 0x1e7a;
    FLD(int32_t, obj, 0x38) = 0x8dc9;
    ScaleAt(obj, 0x34);
    w = (uint32_t)(int32_t)FLD(int16_t, obj, 0x100);
    h = (uint32_t)(int32_t)FLD(int16_t, obj, 0x102);
    FLD(uint32_t, obj, 0x3c) = w - FLD(uint32_t, obj, 0x34) - 1;
    x = FLD(uint32_t, obj, 0x38);
    FLD(uint32_t, obj, 0x40) = h - (x + (uint32_t)Half((int32_t)x)) - 3;
    FLD(uint32_t, obj, 0x44) = w - 1;
    FLD(uint32_t, obj, 0x48) = h - 1;
    FLD(uint32_t, obj, 0x4c) = FLD(uint32_t, obj, 0x3c) + 1;
    FLD(uint32_t, obj, 0x50) = h - (uint32_t)Half((int32_t)FLD(uint32_t, obj, 0x38)) - 2;
    FLD(int32_t, obj, 0x54) = 0x28f;
    FLD(int32_t, obj, 0x58) = 0;
    ScaleAt(obj, 0x54);
}

// FUNCTION: HGSIM 0x10050b66
void __cdecl Sim_10050b66(void *obj)
{
    uint32_t h, x;

    FLD(int32_t, obj, 0x64) = 0x1e7a;
    FLD(int32_t, obj, 0x68) = 0x8dc9;
    FLD(int32_t, obj, 0x5c) = 0;
    FLD(int32_t, obj, 0x60) = 0;
    ScaleAt(obj, 0x64);
    ScaleAt(obj, 0x5c);
    FLD(int32_t, obj, 0x6c) = 0;
    h = (uint32_t)(int32_t)FLD(int16_t, obj, 0x102);
    x = FLD(uint32_t, obj, 0x68);
    FLD(uint32_t, obj, 0x70) = h - (x + (uint32_t)Half((int32_t)x)) - 3;
    FLD(uint32_t, obj, 0x74) = FLD(uint32_t, obj, 0x64);
    FLD(uint32_t, obj, 0x78) = h - 1;
    FLD(uint32_t, obj, 0x5c) = FLD(uint32_t, obj, 0x6c) + 1;
    FLD(uint32_t, obj, 0x60) = h - 2;
    FLD(uint32_t, obj, 0x68) = FLD(uint32_t, obj, 0x78) - FLD(uint32_t, obj, 0x70) - 2;
}
