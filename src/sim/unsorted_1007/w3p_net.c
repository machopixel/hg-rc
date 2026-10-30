#include "w3p.h"
#include "../util/access.h"

typedef int(__cdecl *DpSendFn)(uint32_t session, uint32_t from, uint32_t to, int flag, void *buf, int len);

#define DP_SEND (*SIM_PTR(DpSendFn, 0x1015eab4))

// FUNCTION: HGSIM 0x1007e920
int __cdecl Net_DpSend_1007e920(void *buf, int len, int flag)
{
    uint32_t from = ((uint32_t)len & 0xffff0000u) | *SIM_PTR(u16a, 0x1013acac);

    return DP_SEND(G32(0x1015ecd4), from, 0, flag, buf, len);
}

// FUNCTION: HGSIM 0x1007e950
int __cdecl Net_DpSend_1007e950(uint32_t to, void *buf, int len, int flag)
{
    uint32_t from = ((uint32_t)(uintptr_t)buf & 0xffff0000u) | *SIM_PTR(u16a, 0x1013acac);
    uint32_t dest = ((uint32_t)len & 0xffff0000u) | (to & 0xffffu);

    return DP_SEND(G32(0x1015ecd4), from, dest, flag, buf, len);
}
