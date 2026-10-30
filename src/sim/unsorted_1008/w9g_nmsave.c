#include "w9g.h"
#include "../util/access.h"
#include "../unsorted_1008/w8w.h"
#include "../unsorted_100f/w2e.h"

// FUNCTION: HGSIM 0x10086ebf
void __cdecl Sim_10086ebf(void *obj)
{
    uint8_t rec[0x6c];
    int32_t i;

    if (U32(obj, 0x16) == 0)
        return;
    Sim_10086e27((const char *)GBYTES(0x1014b6f4), rec);
    U32(obj, 0xc) = U32(rec, 0);
    U32(obj, 0x12) = U32(rec, 4);
    U32(obj, 0x126) = U32(rec, 0x10);
    Str_Copy((char *)BYTES(obj) + 0x1e, (const char *)rec + 0x26);
    Mem_Move(BYTES(obj) + 0x12a, rec + 0x14, 0x12);
    for (i = 0; i < S32(obj, 0x12); i++)
        U16(U32(obj, 0x16), (uint32_t)i * 2u) = U16(U32(rec, 0x10), (uint32_t)i * 0x1eu + 0x18u);
    U16(obj, 0x10) = U16(U32(obj, 0x16), U32(rec, 8) * 2u);
}
