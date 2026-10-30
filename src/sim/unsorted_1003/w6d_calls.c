#include "w6d.h"
#include "../unsorted_1008/w3r.h"
#include "../unsorted_1009/w2e.h"
#include "../unsorted_1009/w3o.h"
#include "../unsorted_100a/w5y.h"
#include "../util/util.h"
#include "../util/w3p.h"
#include "../view/view.h"
#include "../view/w2d.h"
#include "../view/w2l.h"
#include "../util/access.h"

static float BitsFloat(uint32_t u)
{
    union {
        uint32_t u;
        float f;
    } v;

    v.u = u;
    return v.f;
}

static uint32_t FloatBits(float f)
{
    union {
        float f;
        uint32_t u;
    } v;

    v.f = f;
    return v.u;
}

// FUNCTION: HGSIM 0x10031ed3
int __cdecl Sim_10031ed3(void *obj, uint32_t b, void *seg)
{
    uint8_t work[0x38];
    uint8_t found[0x38];
    uint32_t best = 0x7f7fc99eu;
    int i;

    Sim_1009cfbd(work, seg);
    for (i = 0; i < 4; i++) {
        if (W6D_CALL_10031ce0(U32(obj, 0x1c + i * 4), b, work) != 0) {
            float length = Sim_1009cbde(work);

            if (X87_LessOrUnordered(length, BitsFloat(best))) {
                best = FloatBits(length);
                Sim_1009cfbd(found, work);
            }
            Sim_1009cfbd(work, seg);
        }
    }
    if (X87_LessOrUnordered(BitsFloat(best), GF32(0x101112ec))) {
        Sim_1009cfbd(seg, found);
        return 1;
    }
    return 0;
}

// FUNCTION: HGSIM 0x100345fc
void __cdecl Sim_100345fc(void)
{
    uint32_t p;

    for (p = 0xb0f00; (int32_t)p < 0xb0fa0; p += 4)
        *(u32a *)(uintptr_t)p &= 0xff00ff00u;
}

// FUNCTION: HGSIM 0x10034cca
void __cdecl Sim_10034cca(void)
{
    uint32_t start = 0;
    uint32_t segment = 0;
    uint32_t old_protect = 0;
    uint32_t size = W6D_CALL_100f9eb7(&start, &segment);
    uint32_t protect = (SIM_API(W6dGetVersionFn, W6D_IAT_GETVERSION)() & 0x80000000u) != 0 ? 0x04u : 0x40u;

    if (SIM_API(W6dVirtualProtectFn, W6D_IAT_VIRTUALPROTECT)((void *)(uintptr_t)start, size, protect, &old_protect) == 0)
        W6D_CALL_100bc280_2(0x4d, 0);
    W6D_CALL_100a8816(0x400, 0xbb8);
    G32(0x1017d140) = 0xb54;
    Sim_100883a2();
    G32(0x1014cb3c) = (uint32_t)(uintptr_t)SIM_PTR(uint8_t, 0x10001122);
    G32(0x1014cb40) = (uint32_t)(uintptr_t)SIM_PTR(uint8_t, 0x10001e42);
    G32(0x1014cb44) = (uint32_t)(uintptr_t)SIM_PTR(uint8_t, 0x1000318e);
    G32(0x1014cb48) = (uint32_t)(uintptr_t)SIM_PTR(uint8_t, 0x10003a08);
    G32(0x1014cb4c) = (uint32_t)(uintptr_t)SIM_PTR(uint8_t, 0x10002608);
    G32(0x10138c84) = 0xff;
    if (G32(0x1014cb04) != 0 || G32(0x1014cb08) != 0)
        G32(0x1014cb18) = 0;
    G32(0x1014caf8) = G32(0x1014caf8) | 8;
}

// FUNCTION: HGSIM 0x10034dc2
void __cdecl Sim_10034dc2(void)
{
    uint32_t k = 0;
    uint32_t cell;

    if (G32(0x1017c8cc) == 0)
        return;
    Sim_10087c86();
    for (cell = G32(0x1036f2a0); cell != 0; cell = G32(0x1036f2a0u + k * 4u)) {
        uint32_t node;
        uint32_t next;

        k++;
        for (node = U32(cell, 8); node != 0; node = next) {
            if ((U16(node, 2) & 0xf0) == 0x90) {
                G32(0x1015e544) = Sim_100096ee((void *)(uintptr_t)node);
                Sim_100aa4a2((void *)(uintptr_t)G32(0x1015e544));
                next = U32(cell, 8);
            } else {
                next = U32(node, 8);
                if ((U16(node, 2) & 0xf0) == 0x70) {
                    Sim_1008a40a((void *)(uintptr_t)node);
                    Sim_1008a37f((void *)(uintptr_t)node);
                }
                if (U32(node, 0x34) == 4)
                    Sim_1008a37f((void *)(uintptr_t)node);
            }
        }
    }
}
