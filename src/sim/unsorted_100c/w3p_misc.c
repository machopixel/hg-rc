#include "w3p.h"
#include "../util/access.h"
#include "../util/w3p.h"

// FUNCTION: HGSIM 0x100c05cb
float __cdecl Sim_100c05cb(const void *obj)
{
    float result = 5000.0f; /* dword 0x459c4000 */

    if (obj != 0) {
        const uint8_t *block = BYTES(U32(obj, 0x28));

        if (S32(block, 0x4fc) != -1) {
            const uint8_t *rec = BYTES(U32(block, 0x84) + (uint32_t)S32(block, 0x4fc) * 0x128u);
            const uint8_t *t = GBYTES(0x10145508) + U32(rec, 4) * 0xd6u;

            result = (float)(((double)F32(t, 0x50) + (double)F32(t, 0x4c)) * (double)GF32(0x1012bb28));
        }
    }
    return result;
}

// FUNCTION: HGSIM 0x100c351c
float __cdecl Math_ClampSymmetric_100c351c(float x, float limit)
{
    if (x > limit)
        x = limit;
    else if (-limit > x)
        x = -limit;
    return x;
}
