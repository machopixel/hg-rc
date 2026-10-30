#include <string.h>
#include "w9d.h"
#include "../unsorted_1000/w5z.h"
#include "../util/access.h"

#define BLOCK GBYTES(0x10152b94)             /* the decoded block */
#define SCRATCH GBYTES(0x10152794)           /* the expansion scratch, 0x400 bytes before the block */
#define DELTA SIM_PTR(u32a, 0x10152f95)      /* the delta entries (up to 16) */

/*
 * 0x100e440b (buffer in EDI, length in ECX): every value becomes four: v, (m + v) / 2, m, (m + w) / 2 with
 * m = (v + w) / 2 and w the next value. The scratch is written and the block read in the original's order
 * (they overlap for lengths above 0x400).
 */
static void W9d_Expand4(uint32_t len)
{
    uint32_t last = len - 1;
    uint32_t i = 0;
    uint32_t k;

    do {
        uint32_t v, w, m;

        k = i >> 2;
        v = U8(BLOCK, k);
        U8(SCRATCH, i) = (uint8_t)v;
        w = U8(BLOCK, k + 1);
        m = (v + w) >> 1;
        U8(SCRATCH, i + 2) = (uint8_t)m;
        U8(SCRATCH, i + 1) = (uint8_t)((m + v) >> 1);
        w = U8(BLOCK, k + 1);
        U8(SCRATCH, i + 3) = (uint8_t)((m + w) >> 1);
        i += 4;
    } while (i < last);
    for (k = 0; k < len; k++)
        U8(BLOCK, k) = U8(SCRATCH, k);
}

/* 0x100e4463 (buffer in EDI, length in ECX): every value becomes two: v, (v + w) / 2. */
static void W9d_Expand2(uint32_t len)
{
    uint32_t last = len - 1;
    uint32_t i = 0;
    uint32_t k;

    do {
        uint32_t v, w;

        k = i >> 1;
        v = U8(BLOCK, k);
        U8(SCRATCH, i) = (uint8_t)v;
        w = U8(BLOCK, k + 1);
        U8(SCRATCH, i + 1) = (uint8_t)((v + w) >> 1);
        i += 2;
    } while (i < last);
    for (k = 0; k < len; k++)
        U8(BLOCK, k) = U8(SCRATCH, k);
}

/* `n` values from codes of `bits` bits (low bits first, `per` codes in a byte); the clamped running value. */
static const uint8_t *W9d_DecodeCodes(const uint8_t *src, uint32_t n, uint32_t bits, uint32_t mask, int32_t *value)
{
    uint32_t per = 8 / bits;
    uint32_t i = 0;
    int32_t v = *value;

    do {
        uint32_t code = *src++;
        uint32_t j;

        for (j = 0; j < per; j++) {
            v = (int32_t)(DELTA[code & mask] + (uint32_t)v);
            code >>= bits;
            if (v > 0x7f)
                v = 0x7f;
            else if (v < -0x7f)
                v = -0x7f;
            U8(BLOCK, i) = (uint8_t)(v + 0x80);
            i++;
        }
    } while (i < n);
    *value = v;
    return src;
}

// FUNCTION: HGSIM 0x100e41cc
uint8_t *__cdecl Sim_100e41cc(uint8_t *src, uint8_t *dst, uint32_t count, uint32_t len, int32_t *state)
{
    do {
        uint32_t rate = src[0] >> 6;
        uint32_t coding = src[0] & 0xf;
        uint32_t n = len;
        int32_t value = *state;
        uint32_t i;

        if (rate == 1)
            n = len >> 1;
        else if (rate == 2)
            n = len >> 2;
        src++;
        switch (coding) {
        case 0:
            memset(BLOCK, 0x80, n);
            value = 0;
            break;
        case 1:
            break;
        case 2:
            DELTA[0] = (uint32_t)src[0] * 2 - 0x80;
            DELTA[1] = (uint32_t)src[1] * 2 - 0x80;
            src = (uint8_t *)W9d_DecodeCodes(src + 2, n, 1, 1, &value);
            break;
        case 3:
            for (i = 0; i < 4; i++)
                DELTA[i] = (uint32_t)src[i] * 2 - 0x80;
            src = (uint8_t *)W9d_DecodeCodes(src + 4, n, 2, 3, &value);
            break;
        case 4:
            for (i = 0; i < 16; i++)
                DELTA[i] = (uint32_t)src[i] * 2 - 0x80;
            src = (uint8_t *)W9d_DecodeCodes(src + 16, n, 4, 0xf, &value);
            break;
        case 5:
            i = 0;
            do {
                U8(BLOCK, i) = *src++;
                i++;
            } while (--n != 0);
            value = (int32_t)U8(BLOCK, i - 1) - 0x80;
            break;
        default:
            return 0;
        }
        if (rate == 1)
            W9d_Expand2(len);
        else if (rate == 2)
            W9d_Expand4(len);
        *state = value;
        memcpy(dst, BLOCK, len);
        dst += len;
    } while (--count != 0);
    return src;
}

// FUNCTION: HGSIM 0x100e3c6a
void __cdecl Sim_100e3c6a(void *obj)
{
    Sim_10009741(obj);
}
