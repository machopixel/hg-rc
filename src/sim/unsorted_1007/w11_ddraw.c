#include "w11.h"
#include "../simmem.h"
#include "../util/access.h"

typedef uint32_t(__stdcall *W11DdMethod0Fn)(uint32_t self);
typedef uint32_t(__stdcall *W11DdMethod2Fn)(uint32_t self, uint32_t a, uint32_t b);

/* method at byte offset off of the COM object whose pointer is the dword at address slot */
#define W11_DD_METHOD(type, slot, off) ((type)(uintptr_t)U32(BYTES(U32(BYTES(G32(slot)), 0)), (off)))

static void W11_Release(uint32_t slot)
{
    W11_DD_METHOD(W11DdMethod0Fn, slot, 8)(G32(slot));
}

// FUNCTION: HGSIM 0x1007f9da
void __cdecl Sim_1007f9da(void)
{
    U32(BYTES(G32(0x1015e698)), 0) = 0;
    if (G32(0x1017b394) != 0) {
        W11_Release(0x1017b394);
        G32(0x1017b394) = 0;
        G32(0x1017b398) = 0;
        G32(0x1017b3a0) = 0;
        if (U32(BYTES(G32(0x1015e68c)), 0) == 0)
            G32(0x1017b39c) = 0;
    }
    if (G32(0x1017b39c) != 0 && (G32(0x109e3d14) != 0x140 || G32(0x109e3d18) != 0xc8)) {
        W11_Release(0x1017b39c);
        G32(0x1017b39c) = 0;
    }
}

// FUNCTION: HGSIM 0x1007f950
void __cdecl Sim_1007f950(void)
{
    G32(0x1017b3b0) = 0;
    if (G32(0x1017b390) == 0)
        return;
    Sim_1007f9da();
    W11_DD_METHOD(W11DdMethod2Fn, 0x1017b390, 0x50)(G32(0x1017b390), G32(0x1017b8f0), 8);
    W11_DD_METHOD(W11DdMethod0Fn, 0x1017b390, 0x4c)(G32(0x1017b390));
    if (G32(0x1017b3a8) != 0) {
        W11_Release(0x1017b3a8);
        G32(0x1017b3a8) = 0;
    }
    W11_Release(0x1017b390);
    G32(0x1017b390) = 0;
}
