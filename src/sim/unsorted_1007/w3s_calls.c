#include "w3s.h"
#include "../util/access.h"

static void CallMethod(uint32_t objectCell)
{
    ((void(__cdecl *)(uint32_t, int))(uintptr_t)U32(G32(objectCell), 0x11c))(G32(objectCell), 0);
}

// FUNCTION: HGSIM 0x100780cf
void __cdecl Sim_100780cf(void)
{
    CallMethod(0x109e8928);
    CallMethod(0x109e8938);
    CallMethod(0x109e890c);
    CallMethod(0x109e88e8);
    CallMethod(0x109e894c);
    CallMethod(0x109e8950);
    CallMethod(0x109e8954);
}

typedef int(__cdecl *ReceiveFn)(uint32_t, uint32_t *, uint32_t *, uint32_t, uint32_t, uint32_t *);

// FUNCTION: HGSIM 0x1007e984
uint32_t __cdecl Sim_1007e984(uint32_t *out)
{
    uint32_t a = 0, b = 0;
    int result = 0;
    int i;

    *out = 0x100;
    for (i = 0; i < 3; i++) {
        result = ((ReceiveFn)(uintptr_t)G32(0x1015eab8))(G32(0x1015ecd4), &a, &b, G32(0x10372d98), G32(0x1015d008), out);
        if (result == 0)
            return (uint16_t)a;
    }
    *out = 0;
    return ((uint32_t)result & 0xffff0000u) | 0xfa01u;
}
