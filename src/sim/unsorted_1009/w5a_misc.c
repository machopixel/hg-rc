#include "w5a.h"
#include "../util/access.h"

// FUNCTION: HGSIM 0x10098f30
void __cdecl Sim_10098f30(uint32_t a, uint32_t b, uint32_t c, uint32_t d)
{
    int low = (int)(a & 0xf);
    int value = (int)(a & 0xff0) >> 4;

    if (low == 0 || low == 3) {
        if ((G32(0x1014caf8) & 1) != 0)
            W5A_CALL_10098f8c(value, b, c, d, 0);
    }
}

// FUNCTION: HGSIM 0x1009d8b9
void __cdecl Sim_1009d8b9(const uint32_t *p)
{
    uint32_t q;
    int n;

    if (p == 0 || p[0] == 0)
        return;
    q = p[0];
    n = S32(q, 4);
    if (n > 0x19)
        return;
    SIM_PTR(u32a, 0x1017cf78)[n] = W5A_CALL_1009db70(U32(q, 0x48));
}
