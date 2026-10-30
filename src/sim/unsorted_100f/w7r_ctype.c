#include "w7r.h"
#include "w6h.h"
#include "../util/access.h"

#define W7R_CTYPE() G32(0x10159450)

// FUNCTION: HGSIM 0x100ff280
int __cdecl Crt_ToLower(int c)
{
    uint8_t in[4];
    uint8_t out[4];
    int n;
    int got;

    if (G32(0x1017e950) == 0) {
        if (c >= 0x41 && c <= 0x5a)
            return c + 0x20;
        return c;
    }
    if (c < 0x100) {
        int upper;

        if (GS32(0x1015965c) > 1)
            upper = Crt_IsCType(c, 1);
        else
            upper = U8(W7R_CTYPE() + (uint32_t)c * 2u, 0) & 1;
        if (upper == 0)
            return c;
    }
    if (U8(W7R_CTYPE() + (((uint32_t)c >> 8) & 0xffu) * 2u, 1) & 0x80) {
        in[0] = (uint8_t)((uint32_t)c >> 8);
        in[1] = (uint8_t)c;
        in[2] = 0;
        n = 2;
    } else {
        in[0] = (uint8_t)c;
        in[1] = 0;
        n = 1;
    }
    got = W7R_FN_101053C0(G32(0x1017e950), 0x100, in, n, out, 3, 0);
    if (got == 0)
        return c;
    if (got == 1)
        return out[0];
    return out[0] | (out[1] << 8);
}
