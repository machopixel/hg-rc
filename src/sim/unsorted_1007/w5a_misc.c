#include "w5a.h"
#include "../util/access.h"
#include "../win/w4u_api.h"

// FUNCTION: HGSIM 0x10074c8f
void __cdecl Sim_10074c8f(void)
{
    uint32_t node = G32(0x1015dc14);
    uint32_t freed;

    while (node != 0) {
        W5A_CALL_10075124(node);
        freed = node;
        node = U32(node, 0x14);
        W4u_HeapFree(freed);
    }
    G32(0x1015dc14) = 0;
    G32(0x1015dc10) = 0;
}

// FUNCTION: HGSIM 0x1007d8ba
int __cdecl Sim_1007d8ba(uint32_t unused, uint8_t *obj)
{
    U32(obj, 0x58) = 2;
    U32(obj, 0x5c) = 3;
    W5A_SPRINTF((char *)obj, SIM_PTR(char, 0x10149ce0));
    W5A_SPRINTF((char *)obj + 0xc, (const char *)(uintptr_t)G32(0x101c49d0));
    W5A_SPRINTF((char *)obj + 0x4c, SIM_PTR(char, 0x10149ce8));
    U32(obj, 0x60) = (uint32_t)(uintptr_t)SIM_PTR(char, 0x10179d38);
    U32(obj, 0x64) = (uint32_t)(uintptr_t)SIM_PTR(char, 0x10179d50);
    U32(obj, 0x68) = (uint32_t)(uintptr_t)SIM_PTR(char, 0x10179d40);
    U32(obj, 0x6c) = (uint32_t)(uintptr_t)SIM_PTR(char, 0x10179d58);
    U32(obj, 0x70) = 0;
    return 0;
}
