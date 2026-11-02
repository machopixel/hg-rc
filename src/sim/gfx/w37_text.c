#include "w37.h"
#include "w37_int.h"
#include "../util/access.h"

// FUNCTION: HGSIM 0x100ecd13
int32_t __cdecl Sim_100ecd13(const uint8_t *pane, int32_t x, int32_t y, const uint8_t *font, int32_t code,
                             const uint8_t *lookaside)
{
    const uint8_t *window = (const uint8_t *)(uintptr_t)U32(pane, 0);
    int32_t pitch, cx0, cy0, cx1, cy1, d;
    uint32_t height, width, skip, n, rows, k;
    const uint8_t *src;
    uint8_t *dst;

    if (S32(window, 4) < 0 || S32(window, 8) < 0)
        return -1;
    pitch = ADD(S32(window, 4), 1);
    cx0 = S32(pane, 4) > 0 ? S32(pane, 4) : 0;
    cy0 = S32(pane, 8) > 0 ? S32(pane, 8) : 0;
    cx1 = S32(pane, 0xc) < S32(window, 4) ? S32(pane, 0xc) : S32(window, 4);
    cy1 = S32(pane, 0x10) < S32(window, 8) ? S32(pane, 0x10) : S32(window, 8);
    if (cx1 < cx0 || cy1 < cy0)
        return -2;
    x = ADD(x, S32(pane, 4));
    y = ADD(y, S32(pane, 8));
    height = U32(font, 8);
    src = font + U32(font, 0x10 + 4 * code);
    width = U32(src, 0);
    if (width == 0)
        return 0;
    src += 4;
    n = width;
    d = (int32_t)((uint32_t)cx1 + 1 - n - (uint32_t)x);
    if (d < 0) {
        if (SUM_LE0(n, d))
            return (int32_t)width;
        n += (uint32_t)d;
    }
    d = DIFF(x, cx0);
    if (d < 0) {
        if (SUM_LE0(n, d))
            return (int32_t)width;
        n += (uint32_t)d;
        src -= d;
        x = DIFF(x, d);
    }
    rows = height;
    d = (int32_t)((uint32_t)cy1 + 1 - rows - (uint32_t)y);
    if (d < 0) {
        if (SUM_LE0(rows, d))
            return (int32_t)width;
        rows += (uint32_t)d;
    }
    d = DIFF(y, cy0);
    if (d < 0) {
        if (SUM_LE0(rows, d))
            return (int32_t)width;
        rows += (uint32_t)d;
        y = DIFF(y, d);
        src -= (uint32_t)d * width;
    }
    dst = (uint8_t *)(uintptr_t)(U32(window, 0) + (uint32_t)y * (uint32_t)pitch + (uint32_t)x);
    skip = width - n;
    if (lookaside == 0) {
        do {
            for (k = 0; k < n; k++)
                *dst++ = *src++;
            src += skip;
            dst += (uint32_t)pitch - n;
        } while (--rows != 0);
        return (int32_t)(skip + n);
    }
    if (n == 0)
        return (int32_t)width;
    do {
        for (k = 0; k < n; k++) {
            uint8_t c = lookaside[*src++];

            if (c != 0xff)
                *dst = c;
            dst++;
        }
        src += skip;
        dst += (uint32_t)pitch - n;
    } while (--rows != 0);
    return (int32_t)(skip + n);
}

// FUNCTION: HGSIM 0x100ecea6
int32_t __cdecl Sim_100ecea6(const uint8_t *pane, int32_t x, int32_t y, const uint8_t *font, const uint8_t *text,
                             const uint8_t *lookaside)
{
    int32_t w;

    do {
        w = Sim_100ecd13(pane, x, y, font, *text, lookaside);
        x += w;
        text++;
    } while (*text != 0);
    return w;
}
