#include "w3q.h"
#include "../view/view.h"
#include "../util/access.h"

typedef void(__cdecl *Method)(void *self, uint32_t arg);
typedef void(__cdecl *MethodNoArg)(void *self);

#define WIDGET(n) ((uint8_t *)(uintptr_t)G32(0x109e88e0 + (uint32_t)(n) * 4))
#define METHOD(obj, off) (*(Method *)((obj) + (off)))

// FUNCTION: HGSIM 0x1002b68f
void __cdecl Sim_1002b68f(const void *owner)
{
    int i;
    int k = 3;
    uint8_t *rec;
    uint8_t *recs = BYTES(U32(owner, 0x84));

    for (i = 0; i < 8; i++)
        U32(recs, (uint32_t)i * 0x128 + 0x104) = (uint32_t)i;
    rec = recs;
    for (i = 0; i < 8; i++) {
        uint8_t *obj = WIDGET(k);
        int32_t *area;

        METHOD(obj, 0x114)(obj, (uint32_t)(uintptr_t)(GBYTES(0x10145592) + U32(rec, 4) * 0xd6));
        METHOD(obj, 0x110)(obj, (uint32_t)i);
        U32(obj, 0x138) = (uint32_t)(uintptr_t)SIM_PTR(uint8_t, 0x100026b2);
        U32(obj, 0x13c) = (uint32_t)(uintptr_t)SIM_PTR(uint8_t, 0x10002c34);
        if (G32(0x101c64f0) != 0) {
            U32(obj, 0xd4) = 0x11ec;
            U32(obj, 0xd8) = 0x6666;
            U32(obj, 0xdc) = 0x11ec;
            U32(obj, 0xe0) = 0x51f;
        } else {
            U32(obj, 0xd4) = 0xccd;
            U32(obj, 0xd8) = 0x199a;
            U32(obj, 0xdc) = 0xb333;
            U32(obj, 0xe0) = 0x199a;
        }
        U32(obj, 0xe4) = 0x199a;
        U32(obj, 0xe8) = 0xcccd;
        area = (int32_t *)(uintptr_t)U32(obj, 0xec);
        Rect_ScalePoint(area, (int32_t *)(obj + 0xd4), (int32_t *)(obj + 0xd4));
        area = (int32_t *)(uintptr_t)U32(obj, 0xec);
        Rect_ScalePoint(area, (int32_t *)(obj + 0xdc), (int32_t *)(obj + 0xdc));
        area = (int32_t *)(uintptr_t)U32(obj, 0xec);
        Rect_ScalePoint(area, (int32_t *)(obj + 0xe4), (int32_t *)(obj + 0xe4));
        k++;
        if (U32(rec, 0) == 0)
            U32(rec, 4) = 0xffffffffu;
        rec += 0x128;
    }
}

// FUNCTION: HGSIM 0x1002c113
void __cdecl Sim_1002c113(void)
{
    uint32_t entry = PTRTAB(G32(0x1017b8c8));
    int n;

    Sim_1002b68f((const void *)(uintptr_t)U32(entry, 0x28));
    for (n = 0; n < 0x1e; n++) {
        uint8_t *obj = WIDGET(n);

        METHOD(obj, 0x10c)(obj, G32(0x101381d0 + (uint32_t)n * 4));
        obj = WIDGET(n);
        METHOD(obj, 0x124)(obj, G32(0x10138158 + (uint32_t)n * 4));
        obj = WIDGET(n);
        METHOD(obj, 0x12c)(obj, 0);
        if (G32(0x109e8960 + (uint32_t)n * 4) != 0) {
            obj = WIDGET(n);
            (*(MethodNoArg *)(obj + 0x130))(obj);
        }
    }
}
