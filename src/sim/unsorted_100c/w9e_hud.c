#include "w9e.h"
#include "w7q.h"
#include "../unsorted_1004/fns.h"
#include "../unsorted_1008/w8x.h"
#include "../unsorted_100b/w6g.h"
#include "../unsorted_100f/w8z.h"
#include "../util/access.h"

#define STR_AT(addr) SIM_PTR(const char, addr)
#define TEXT_COLOUR (*SIM_PTR(uint8_t, 0x109b992e))
#define PANE(hud) BYTES(U32(hud, 0xec))

/* Draws text at the position with the font, in the routine of the display mode. */
static void W9e_Text(const uint8_t *hud, const int32_t *pos, void *font, const char *text)
{
    if (G32(0x101668e8) != 0)
        W9E_CALL_100f2f01(PANE(hud), pos[0], pos[1], font, text, GBYTES(0x109b9920));
    else
        W9E_CALL_100f1e10(PANE(hud), pos[0], pos[1], font, text, GBYTES(0x109b9920));
}

/* A text in colour 0xc, leaving the colour 9. */
static void W9e_Line(const uint8_t *hud, const int32_t *pos, void *font, const char *text)
{
    TEXT_COLOUR = 0xc;
    W9e_Text(hud, pos, font, text);
    TEXT_COLOUR = 9;
}

// FUNCTION: HGSIM 0x100ce983
void __cdecl Sim_100ce983(uint8_t *hud)
{
    int32_t pos[2];
    char text[0x28];
    uint8_t colour = 0xc;
    const char *label = STR_AT(0x1017e5f4);
    void *font;
    int32_t height, half, target, who;

    if (S16(hud, 4) == 0)
        return;
    if (G32(0x1015f208) == 0 && GS32(0x10151d50) == -2)
        return;
    font = W9E_CALL_100ba7d5((char *)BYTES(G32(0x109b9900)), (char *)BYTES(G32(0x1014cec4)), 0);
    if (font == 0)
        return;
    height = W9E_CALL_100ecce0(font);
    half = height / 2;
    pos[0] = S32(U32(hud, 0xf0), 0);
    pos[1] = S32(U32(hud, 0xf0), 4);
    if (G32(0x1015f208) == 1) {
        G32(0x10179ca0) = 0;
        G32(0x10179ca4) = 0;
        if (Sim_10052be7(GS32(0x1015ef18)) != 0)
            colour = 7;
        if ((S16(PTRTAB(G32(0x1015ef18)), 0x1c) & 6) != 0)
            label = (const char *)BYTES(G32(0x101c4c44));
        Crt_SPrintf(text, STR_AT(0x10151e28), BYTES(G32(0x101c4c00)), BYTES(PTRTAB(G32(0x1015ef18))) + 0x124, label);
        TEXT_COLOUR = colour;
        W9e_Text(hud, pos, font, text);
        TEXT_COLOUR = 9;
    } else if (G32(0x1015f208) == 2) {
        G32(0x10179ca0) = 0;
        G32(0x10179ca4) = 0;
        TEXT_COLOUR = 0x1c;
        W9e_Text(hud, pos, font, (const char *)BYTES(G32(0x101c4c08)));
        TEXT_COLOUR = 9;
    } else if (G32(0x1015f208) == 3) {
        W9e_Line(hud, pos, font, (const char *)BYTES(G32(0x101c4c10)));
    }
    pos[1] += height + half;
    if (GS32(0x10151d50) == -2) {
        Sim_100bb2b0((const uint32_t *)BYTES(G32(0x109b9900)), (const char *)BYTES(G32(0x1014cec4)));
        return;
    }
    target = GS32(0x10151d50);
    if (target == -1) {
        W9e_Line(hud, pos, font, (const char *)BYTES(G32(0x101c4c18)));
        Sim_1008d6f7(PANE(hud), BYTES(G32(0x101c4c18)), pos[0], pos[1], font, 0x18);
        pos[1] += height + half;
        if (U8(G32(0x1017b8c4), 0xa) == 0) {
            W9e_Line(hud, pos, font, (const char *)BYTES(G32(0x101c4c0c)));
        } else {
            W9e_Line(hud, pos, font, (const char *)BYTES(G32(0x101c4c24)));
            pos[1] += height;
            W9e_Line(hud, pos, font, (const char *)BYTES(G32(0x101c4c30)));
            pos[1] += height;
            W9e_Line(hud, pos, font, (const char *)BYTES(G32(0x101c4bfc)));
            pos[1] += height;
            W9e_Line(hud, pos, font, (const char *)BYTES(G32(0x101c4c04)));
        }
        pos[1] += height + half;
    } else if (target >= 0 && target <= 7) {
        who = target == GS32(0x1017b8c8) ? -2 : target;
        if (who < 0)
            return;
        if (PTRTAB(who) == 0)
            return;
        if (S32(PTRTAB(who), 0) != 1)
            return;
        if ((S16(PTRTAB(who), 0x1c) & 0x4800) != 0)
            return;
        Crt_SPrintf(text, STR_AT(0x10151e30), BYTES(G32(0x101c4c18)), BYTES(PTRTAB(who)) + 0x124);
        W9e_Line(hud, pos, font, text);
        Sim_1008d6f7(PANE(hud), (const uint8_t *)text, pos[0], pos[1], font, 0x18);
        pos[1] += height + half;
        W9e_Line(hud, pos, font, (const char *)BYTES(G32(0x101c4c20)));
        pos[1] += height + half;
    }
    W9e_Line(hud, pos, font, STR_AT(0x1015dba0));
    Sim_1008d8c0(PANE(hud), SIM_PTR(const uint8_t, 0x10151e38), pos[0], pos[1], font, 0x14);
    pos[0] = Sim_100ccb0d(STR_AT(0x1015dba0), font);
    Sim_1008d6f7(PANE(hud), SIM_PTR(const uint8_t, 0x10151e64), pos[0], pos[1], font, 0xc);
    Sim_100bb2b0((const uint32_t *)BYTES(G32(0x109b9900)), (const char *)BYTES(G32(0x1014cec4)));
}
