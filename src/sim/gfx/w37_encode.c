#include "w37.h"
#include "w37_int.h"
#include "../simmem.h"
#include "../util/access.h"

/*
 * The shape encoder's state (DLL data): [0x1015365d] the output (0: only measure), [0x10153661] the pending
 * transparent count, [0x10153665] the current row's start, [0x10153669] the source position after the current
 * packet, [0x1015366d] the output position, [0x10153671] the source position, [0x10153685] / [0x10153689] /
 * [0x1015368d] / [0x10153691] the bounding box (x0, y0, x1, y1).
 */
#define ENC_OUT G32(0x1015365d)
#define ENC_SKIP GS32(0x10153661)
#define ENC_ROW G32(0x10153665)
#define ENC_END G32(0x10153669)
#define ENC_DST G32(0x1015366d)
#define ENC_SRC G32(0x10153671)
#define ENC_X0 GS32(0x10153685)
#define ENC_Y0 GS32(0x10153689)
#define ENC_X1 GS32(0x1015368d)
#define ENC_Y1 GS32(0x10153691)

/* The pending transparent cells as skip packets {1, count} of at most 255. */
static void W37_EncodeSkips(uint32_t *dst, uint32_t *src)
{
    int32_t n = ENC_SKIP, k;

    if (n == 0)
        return;
    do {
        k = n < 0xff ? n : 0xff;
        n -= k;
        if (ENC_OUT != 0) {
            BYTES(*dst)[0] = 1;
            BYTES(*dst)[1] = (uint8_t)k;
        }
        *dst += 2;
        *src += (uint32_t)k;
    } while (n != 0);
    ENC_SKIP = 0;
}

/* The packet's cell count (the source up to [0x10153669] less `back` cells) and the box's x range. */
static int32_t W37_EncodeSpan(uint32_t src, int32_t back, int32_t xpos, int32_t *grew)
{
    int32_t n = DIFF(DIFF(ENC_END, src), back);
    int32_t x = DIFF(ADD(xpos, src), ENC_ROW);

    if (x < ENC_X0)
        ENC_X0 = x;
    x = DIFF(ADD(x, n), 1);
    *grew = x > ENC_X1;
    if (*grew)
        ENC_X1 = x;
    return n;
}

// FUNCTION: HGSIM 0x100eae93
void __cdecl Sim_100eae93(int32_t mode, int32_t back, int32_t xpos)
{
    uint32_t dst = ENC_DST, src = ENC_SRC;
    int32_t n, k, grew;

    switch (mode) {
    case 0:                             /* row start */
        ENC_SKIP = 0;
        src = ENC_END;
        break;
    case 2:                             /* a run of equal cells: {count * 2, cell} */
        W37_EncodeSkips(&dst, &src);
        n = W37_EncodeSpan(src, back, xpos, &grew);
        if (grew && n == 0)
            break;
        do {
            k = n < 0x7f ? n : 0x7f;
            if (ENC_OUT != 0) {
                BYTES(dst)[0] = (uint8_t)(k + k);
                BYTES(dst)[1] = BYTES(src)[0];
            }
            dst += 2;
            src += (uint32_t)k;
            n -= k;
        } while (n != 0);
        break;
    case 1:                             /* different cells: {count * 2 + 1, cells} */
        W37_EncodeSkips(&dst, &src);
        n = W37_EncodeSpan(src, back, xpos, &grew);
        if (grew && n == 0)
            break;
        do {
            uint32_t j;

            k = n < 0x7f ? n : 0x7f;
            if (ENC_OUT != 0) {
                BYTES(dst)[0] = (uint8_t)(k + k + 1);
                dst++;
                for (j = 0; j < (uint32_t)k; j++)
                    BYTES(dst++)[0] = BYTES(src++)[0];
            } else {
                dst++;
                src += (uint32_t)k;
                dst += (uint32_t)k;
            }
            n -= k;
        } while (n != 0);
        break;
    case 3:                             /* transparent cells, kept pending */
        ENC_SKIP = DIFF(DIFF(ENC_END, src), back);
        break;
    case 4:                             /* row end */
        if (ENC_OUT != 0)
            BYTES(dst)[0] = 0;
        dst++;
        break;
    }
    ENC_DST = dst;
    ENC_SRC = src;
}

// FUNCTION: HGSIM 0x100ead0a
void __cdecl Sim_100ead0a(uint32_t count, uint32_t key, int32_t xpos)
{
    const uint8_t *src = (const uint8_t *)(uintptr_t)ENC_ROW;
    uint8_t cur, k = (uint8_t)key;
    uint32_t n = count;
    int32_t state;
    int differ;

/* The next cell: `differ` when it differs from the last one, which it then becomes. */
#define NEXT()                                                                                                     \
    do {                                                                                                           \
        uint8_t b_ = *src++;                                                                                       \
        n--;                                                                                                       \
        differ = b_ != cur;                                                                                        \
        cur = b_;                                                                                                  \
    } while (0)
#define EMIT(mode_, back_)                                                                                         \
    do {                                                                                                           \
        ENC_END = (uint32_t)(uintptr_t)src;                                                                        \
        Sim_100eae93((mode_), (back_), xpos);                                                                      \
    } while (0)

    ENC_END = (uint32_t)(uintptr_t)src;
    Sim_100eae93(0, 0, xpos);
    state = 5;
    if (n == 0)
        goto end;
    cur = *src++;
    n--;
    if (cur == k)
        goto skip;
literal:
    state = 1;
    if (n == 0)
        goto end;
    NEXT();
    if (!differ)
        goto run;
    if (cur == k) {
        EMIT(1, 1);
        goto skip;
    }
    for (;;) {
        if (n == 0)
            goto end;
        NEXT();
        if (cur == k) {
            EMIT(1, 1);
            goto skip;
        }
        if (differ)
            continue;
        if (n == 0)
            goto end;
        NEXT();
        if (cur == k) {
            EMIT(1, 1);
            goto skip;
        }
        if (differ)
            continue;
        EMIT(1, 3);                     /* three equal cells end the literal */
        break;
    }
run:
    state = 2;
    if (n == 0)
        goto end;
    NEXT();
    if (!differ)
        goto run;
    EMIT(2, 1);
    if (cur != k)
        goto literal;
skip:
    state = 3;
    if (n == 0)
        goto end;
    NEXT();
    if (!differ)
        goto skip;
    EMIT(3, 1);
    goto literal;
end:
    EMIT(state, 0);
    Sim_100eae93(4, 0, xpos);
#undef NEXT
#undef EMIT
}

// FUNCTION: HGSIM 0x100ea9f5
int32_t __cdecl Sim_100ea9f5(const uint8_t *pane, uint32_t key, int32_t x, int32_t y, uint8_t *out)
{
    const uint8_t *window = (const uint8_t *)(uintptr_t)U32(pane, 0);
    int32_t pitch, cx0, cy0, cx1, cy1, width, row, left, k;
    uint8_t *buffer, *p;

    if (S32(window, 4) < 0 || S32(window, 8) < 0)
        return -1;
    pitch = ADD(S32(window, 4), 1);
    cx0 = S32(pane, 4) > 0 ? S32(pane, 4) : 0;
    cy0 = S32(pane, 8) > 0 ? S32(pane, 8) : 0;
    cx1 = S32(pane, 0xc) < S32(window, 4) ? S32(pane, 0xc) : S32(window, 4);
    cy1 = S32(pane, 0x10) < S32(window, 8) ? S32(pane, 0x10) : S32(window, 8);
    if (cx1 < cx0 || cy1 < cy0)
        return -2;
    buffer = (uint8_t *)(uintptr_t)U32(window, 0);
    ENC_OUT = (uint32_t)(uintptr_t)out;
    if (out != 0) {
        U32(out, 0) = (U32(pane, 0xc) - U32(pane, 4)) << 16 | (uint16_t)(U16(pane, 0x10) - U16(pane, 8));
        U32(out, 4) = (uint32_t)x << 16 | (uint16_t)y;
        U32(out, 8) = 0;
        U32(out, 0xc) = 0;
        U32(out, 0x10) = 0xffffffffu;
        U32(out, 0x14) = 0xffffffffu;
    }
    x = ADD(x, S32(pane, 4));
    y = ADD(y, S32(pane, 8));
    width = DIFF(ADD(cx1, 1), cx0);
    ENC_X0 = 0x7fffffff;
    ENC_Y0 = 0x7fffffff;
    ENC_X1 = -0x7fffffff;
    ENC_Y1 = -0x7fffffff;
    /* the box of the cells that are not `key` (repe scasb from each end of every row) */
    ENC_ROW = (uint32_t)(uintptr_t)buffer + (uint32_t)cy0 * (uint32_t)pitch + (uint32_t)cx0;
    for (row = cy0; row <= cy1; row++) {
        p = (uint8_t *)(uintptr_t)ENC_ROW;
        ENC_END = ENC_ROW;
        for (k = 0; k < width && p[k] == (uint8_t)key; k++)
            ;
        if (k < width) {
            left = width - k - 1;       /* the scan's count left */
            if (ENC_Y0 >= row)
                ENC_Y0 = row;
            if (ENC_Y1 <= row)
                ENC_Y1 = row;
            if (DIFF(cx1, left) < ENC_X0)
                ENC_X0 = DIFF(cx1, left);
            ENC_END = ENC_ROW + (uint32_t)width - 1;
            for (k = width - 1; p[k] == (uint8_t)key; k--)
                ;
            if (ADD(cx0, k) > ENC_X1)
                ENC_X1 = ADD(cx0, k);
        }
        ENC_ROW += (uint32_t)pitch;
    }
    if (out != 0) {
        S32(out, 8) = DIFF(ENC_X0, x);
        S32(out, 0xc) = DIFF(ENC_Y0, y);
        S32(out, 0x10) = DIFF(ENC_X1, x);
        S32(out, 0x14) = DIFF(ENC_Y1, y);
    }
    ENC_DST = (uint32_t)(uintptr_t)out + 0x18;
    width = DIFF(ADD(ENC_X1, 1), ENC_X0);
    ENC_ROW = (uint32_t)(uintptr_t)buffer + (uint32_t)ENC_Y0 * (uint32_t)pitch + (uint32_t)ENC_X0;
    for (row = ENC_Y0; row <= ENC_Y1; row++) {
        Sim_100ead0a((uint32_t)width, key, cx0);
        ENC_ROW += (uint32_t)pitch;
    }
    return DIFF(ENC_DST, (uint32_t)(uintptr_t)out);
}
