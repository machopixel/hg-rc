#include "w5c.h"
#include "../view/view.h"
#include "../util/access.h"

// FUNCTION: HGSIM 0x100d255e
void __cdecl Sim_100d255e(void *obj)
{
    uint32_t block[12];

    SIM_CALL_10060656(block, U32(obj, 0x10), U32(obj, 0xc), U32(obj, 0x14), U32(obj, 0), U32(obj, 4), U32(obj, 8));
    Mat3_Transpose(block, (uint32_t *)(BYTES(obj) + 0x54));
    U32(obj, 0x78) = block[9];
    U32(obj, 0x7c) = block[10];
    U32(obj, 0x80) = block[11];
}
