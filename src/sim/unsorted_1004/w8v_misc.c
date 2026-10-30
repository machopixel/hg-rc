#include "w8v.h"
#include "w2h.h"
#include "w4w.h"
#include "../util/access.h"
#include "../render/w2n.h"
#include "../unsorted_1003/w3p.h"
#include "../unsorted_1007/w2e.h"
#include "../unsorted_100f/w2e.h"
#include "../unsorted_100f/w7q.h"

/*
 * Steps the entry `index` of the table at 0x109b94e0 (state +0, frame count +4, ticks per frame +8, start tick
 * +0xc, flags +0x10, offset +0x14, handle +0x18): opens the handle on first use, works out the frame from the
 * tick counter 0x1015dc78 and draws it with 0x100e91dc, or ends the entry (0x100481dd) when flag 8 asks for it.
 * The original's range check of `index` never rejects anything; it is left out.
 */
// FUNCTION: HGSIM 0x10048287
void __cdecl Sim_10048287(uint32_t a, int32_t index, uint32_t c, uint32_t d)
{
    uint8_t *e = BYTES(SIM_PTR(u32a, 0x109b94e0)[index]);
    int32_t t;
    uint16_t frame = 0;

    if (e == 0)
        return;
    if (U32(e, 0) == 1)
        return;
    if (U32(e, 0x18) == 0) {
        U32(e, 0x18) = W8V_CALL_100282d4(G32(0x1014cf8c), U32(e, 0x14) + G32(0x101c64f0), G32(0x1014cec0), 0);
        if (U32(e, 0x18) == 0) {
            Sim_100481dd(index);
            return;
        }
        U32(e, 4) = W8V_CALL_100ed8c3(U32(e, 0x18));
    }
    if (U32(e, 0) == 0)
        U32(e, 0) = 2;
    if (U32(e, 0xc) != 0) {
        t = (int32_t)(G32(0x1015dc78) - U32(e, 0xc)) / S32(e, 8);
        if (t < 0)
            t = 0;
    } else {
        t = 0;
        U32(e, 0xc) = G32(0x1015dc78);
    }
    if (U32(e, 0x10) & 1) {
        if (t >= S32(e, 4)) {
            if (U32(e, 0x10) & 2) {
                frame = (uint16_t)(U32(e, 4) - 1);
                U32(e, 0) = 3;
            } else {
                /* the frame stays unset: the state 1 below never uses it */
                U32(e, 0) = 1;
            }
        } else {
            frame = (uint16_t)t;
        }
    } else {
        frame = (uint16_t)(t % S32(e, 4));
    }
    if (U32(e, 0) != 1)
        W8V_CALL_100e91dc(a, U32(e, 0x18), frame, c, d);
    else if (U32(e, 0x10) & 8)
        Sim_100481dd(index);
}

#define LINE GBYTES(0x1015dba0)
#define LINE_LEN GS32(0x1015f1f4)
#define CHAT_TARGET GS32(0x10151d50)

/*
 * One key of the chat line (text at 0x1015dba0, length at 0x1015f1f4, at most 0x27 characters): backspace,
 * escape (clears the line), return or the keys 'e'/'f' with flag 0x100 in the target -1 mode, flag 0x200 maps
 * the character to upper case and through Sim_10048bd6. A finished line (target -2) goes to Sim_1003fcdc, and a
 * line for everybody (a negative target) is also shown as "<own name>: <text>" with Sim_1007df6c. Returns 1 when
 * the key was used, 0 otherwise.
 */
// FUNCTION: HGSIM 0x10048c84
int __cdecl Chat_HandleKey(uint32_t key)
{
    char text[0x50];
    int32_t target;
    uint8_t c;

    target = G32(0x10151d50) == G32(0x1017b8c8) ? -4 : CHAT_TARGET;
    c = (uint8_t)key;
    if (c < 0x7e && (key & 0x400) != 0)
        return 0;
    if (c == 8) {
        if (LINE_LEN > 0)
            LINE_LEN = LINE_LEN - 1;
        LINE[LINE_LEN] = 0;
        return 1;
    }
    if (c == 0x1b) {
        CHAT_TARGET = -2;
        Mem_Fill(LINE, 0, 0x28);
        LINE_LEN = 0;
        return 1;
    }
    if (c == 0xd) {
        if (CHAT_TARGET >= 0) {
            CHAT_TARGET = -2;
        } else if (CHAT_TARGET == -1) {
            target = -1;
            CHAT_TARGET = -2;
        }
    }
    if (key & 0x100) {
        if (CHAT_TARGET != -1)
            return 0;
        if (c == 0x66) {
            target = -2;
            CHAT_TARGET = -2;
        } else if (c == 0x65) {
            target = -3;
            CHAT_TARGET = -2;
        } else if (c < 0x7e) {
            return 0;
        }
    }
    if (key & 0x200) {
        c = (uint8_t)Crt_ToUpper(c);
        Sim_10048bd6(&c);
    }
    if (CHAT_TARGET == -2) {
        LINE[LINE_LEN] = 0;
        Sim_1003fcdc(target, (const char *)LINE);
        if (target < 0) {
            W8V_SPRINTF(text, SIM_PTR(char, 0x1013b6cc), BYTES(PTRTAB(G32(0x1017b8c8))) + 0x124, LINE);
            Sim_1007df6c(text, (const char *)(uintptr_t)G32(0x109b98c0), 0x2d4, 0x32);
        }
        Mem_Fill(LINE, 0, 0x28);
        LINE_LEN = 0;
        return 1;
    }
    if (c < 0x20)
        return 0;
    if (LINE_LEN >= 0x27)
        return 1;
    LINE[LINE_LEN] = c;
    LINE_LEN = LINE_LEN + 1;
    return 1;
}

/*
 * Fills the 16 x 64 byte table at 0x109b9080: row i maps a 6-bit colour value j to
 * trunc(pow(j / 63, 1 / (i / 16 + 0.5)) * 63) (the constants are the doubles at 0x101115f8..0x10111610).
 */
// FUNCTION: HGSIM 0x1004b710
void __cdecl Gamma_BuildTable(void)
{
    const double *k = SIM_PTR(const double, 0x101115f8);
    double e, x;
    int32_t i, j;

    for (i = 0; i < 0x10; i++) {
        e = (double)i * k[0] + k[1];
        e = k[2] / e;
        for (j = 0; j < 0x40; j++) {
            x = (double)j / k[3];
            GBYTES(0x109b9080)[i * 0x40 + j] = (uint8_t)Render_Ftol(W8V_POW(x, e) * k[3]);
        }
    }
}
