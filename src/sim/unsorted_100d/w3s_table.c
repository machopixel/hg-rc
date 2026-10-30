#include "w3s.h"
#include "../render/w3s_fp.h"
#include "../unsorted_100f/w2e.h"

// FUNCTION: HGSIM 0x100daea4
int __cdecl Sim_100daea4(const void *src)
{
    uint32_t count = (U32(src, 4) - 0x18u) / 12u;
    uint8_t *rec;
    int i;

    if (GS32(0x1015f3ac) >= 0x20 || (int32_t)count > 8)
        return 0;
    rec = GBYTES(0x109b68e0) + (uint32_t)GS32(0x1015f3ac) * 0x74u;
    G32(0x1015f3ac) = G32(0x1015f3ac) + 1;
    Str_CopyN((char *)rec, (const char *)BYTES(src) + 8, 0x10);
    for (i = 0; i < (int32_t)count; i++) {
        F32(rec, 0x10 + (uint32_t)i * 4) = (float)S32(src, 0x18 + i * 0xc);
        F32(rec, 0x30 + (uint32_t)i * 4) = (float)S32(src, 0x1c + i * 0xc);
        F32(rec, 0x50 + (uint32_t)i * 4) = (float)((double)S32(src, 0x20 + i * 0xc) * (double)GF32(0x1012bd88));
    }
    U32(rec, 0x70) = count;
    return 1;
}
