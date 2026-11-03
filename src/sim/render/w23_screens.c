#include "../unsorted_1004/w3p.h"
#include "w23.h"
#include "../util/access.h"
#include "../unsorted_100c/w9g.h"
#include "../unsorted_100c/w5b.h"
#include "../unsorted_100c/w5c.h"
#include "../unsorted_100c/w9c.h"
#include "../unsorted_1008/w8x.h"

#define W23H_LOAD SIM_FN(void *(__cdecl *)(uint32_t, uint32_t, uint32_t), 0x100ba7d5)
#define W23H_RELEASE SIM_FN(void(__cdecl *)(uint32_t, uint32_t), 0x100bb2b0)
#define W23H_SHAPE_ACCEL SIM_FN(void(__cdecl *)(uint32_t, void *, int32_t, int32_t, int32_t), 0x100f0205)
#define W23H_SHAPE_SOFT SIM_FN(void(__cdecl *)(uint32_t, void *, int32_t, int32_t, int32_t), 0x100e91dc)

// FUNCTION: HGSIM 0x100d422f
void __cdecl Sim_100d422f(void *panel)
{
    uint8_t *slot;
    int32_t now;
    void *image;

    if (S16(panel, 4) == 0 || S32(panel, 0xc) < 0)
        return;
    slot = BYTES(U32(U32(PTRTAB(G32(0x1017b8c8)), 0x28), 0x84)) + (uint32_t)S32(panel, 0xc) * 0x128u;
    if (S32(slot, 4) < 0)
        return;
    now = GS32(0x1015dc78);
    if (W23H_LOAD(G32(0x109b9900), G32(0x1014cec4), 0) == 0)
        return;
#ifdef W23H_MUTATE /* the checker's negative check only */
    if (S16(panel, 4) != 0 && now >= S32(panel, 8)) {
#else
    if (S16(panel, 4) != 0 && now > S32(panel, 8)) {
#endif
        image = W23H_LOAD(G32(0x109e84b8), G32(0x1014cec0), 0);
        if (image != 0) {
            W23H_SHAPE_ACCEL(U32(panel, 0xec), image, 0, 0, 0);
            W23H_RELEASE(G32(0x109e84b8), G32(0x1014cec0));
        }
    }
    W23H_RELEASE(G32(0x109b9900), G32(0x1014cec4));
}

#define W23H_FONT_HEIGHT SIM_FN(int32_t(__cdecl *)(void *), 0x100ecce0)
#define W23H_PIECE SIM_FN(uint32_t(__cdecl *)(uint32_t, uint32_t, uint32_t, uint32_t), 0x100652e8)
#define W23H_SPRINTF SIM_FN(int(__cdecl *)(char *, const char *, ...), 0x100fc780)
typedef int(__cdecl *W23hTextFn)(uint32_t surface, int32_t x, int32_t y, void *font, const char *text, void *pal);

/* one text line through the display's text routine */
static void W23h_Text(uint32_t surface, int32_t x, int32_t y, void *font, const char *text)
{
    if (G32(0x101668e8) != 0)
        SIM_FN(W23hTextFn, 0x100f2f01)(surface, x, y, font, text, GBYTES(0x109b9920));
    else
        SIM_FN(W23hTextFn, 0x100f1e10)(surface, x, y, font, text, GBYTES(0x109b9920));
}

// FUNCTION: HGSIM 0x100ce586
void __cdecl Sim_100ce586(void *panel)
{
    char line[0x100];
    int32_t pos[2], height, half, i;
    void *font;
    uint8_t *unit;

    if (S16(panel, 4) == 0 || G32(0x10151d54) == 0)
        return;
    font = W23H_LOAD(G32(0x109b9900), G32(0x1014cec4), 0);
    if (font == 0)
        return;
    height = W23H_FONT_HEIGHT(font);
    pos[0] = S32(U32(panel, 0xf0), 0);
    pos[1] = S32(U32(panel, 0xf0), 4);
    pos[0] = S32(panel, 0xb4);
    pos[1] = S32(panel, 0xb8);
    G32(0x10151d9c) = U32(panel, 0xd0) + G32(0x1017e5e4) + 0x42;
    G32(0x10151db4) = U32(panel, 0xd0) + G32(0x1017e5e4) + 0x42;
    /* 1.2 (0x100b26e6) no longer reads the player's gear block here (1.0 read it unused) */
    W23H_PIECE(0, G32(0x1017e5e4), G32(0x109e8494), (uint32_t)(uintptr_t)GBYTES(0x109e878c));
    if (G32(0x1017e5ec) != 0) {
        GS32(0x1017e5e4) += (int32_t)(G32(0x1017e5e8) * G32(0x1015dc80)) >> 16;     /* imul, sar */
        half = GS32(0x10151d58 + G32(0x101c64f0) * 4u) / 2;
        if (GS32(0x1017e5e4) >= half)
            GS32(0x1017e5e4) = GS32(0x10151d58 + G32(0x101c64f0) * 4u) / 2;
    } else {
#ifdef W23H_MUTATE2 /* the checker's negative check only */
        GS32(0x1017e5e4) += (int32_t)(G32(0x1017e5e8) * G32(0x1015dc80)) >> 16;
#else
        GS32(0x1017e5e4) -= (int32_t)(G32(0x1017e5e8) * G32(0x1015dc80)) >> 16;
#endif
        if (GS32(0x1017e5e4) <= GS32(0x10151d58 + G32(0x101c64f0) * 4u))
            GS32(0x1017e5e4) = GS32(0x10151d58 + G32(0x101c64f0) * 4u);
    }
    W23H_PIECE(0, 0, G32(0x109e8490), U32(panel, 0xec));
    if (G32(0x10151d54) == 1) {
        Sim_100cd70f(panel, pos, (uint32_t)(uintptr_t)font);
    } else {
        for (i = 0; i < GS32(0x1015dc64); i++) {
            unit = BYTES(PTRTAB(i));
            if (unit == 0)
                break;
            if ((S16(unit, 0x1c) & 0x4800) != 0)
                continue;
            W23H_SPRINTF(line, SIM_PTR(const char, 0x10151e20), unit + 0x124,
                         (int)g_w34Rating[i]);
            pos[1] += height;
            *GBYTES(0x109b992e) = 0xc;
            W23h_Text(U32(panel, 0xec), pos[0], pos[1], font, line);
            *GBYTES(0x109b992e) = 9;
        }
    }
    W23H_RELEASE(G32(0x109b9900), G32(0x1014cec4));
}

// FUNCTION: HGSIM 0x100cde94
void __cdecl Sim_100cde94(void *panel)
{
    char line[0x100];
    int32_t pos[2], height, half, i;
    uint8_t *status, *unit;
    void *font;

    line[0] = 0;    /* the original draws this line unwritten for status 1 or above 4 (DECISIONS.md round 23) */
    if (S16(panel, 4) == 0 || G32(0x1017e5e0) == 0)
        return;
    status = GBYTES(0x10473800 + G32(0x1013bb40) * 0x4fc3au);
    font = W23H_LOAD(G32(0x109b9900), G32(0x1014cec4), 0);
    if (font == 0)
        return;
    height = W23H_FONT_HEIGHT(font);
    half = height / 2;
    pos[0] = S32(U32(panel, 0xf0), 0);
    pos[1] = S32(U32(panel, 0xf0), 4);
    if (G32(0x1017e5e0) == 1) {
        *GBYTES(0x109b992e) = 0x14;
        W23h_Text(U32(panel, 0xec), pos[0], pos[1], font, SIM_PTR(const char, G32(0x101c4c4c)));
        *GBYTES(0x109b992e) = 9;
        Sim_1008d6f7(BYTES(U32(panel, 0xec)), SIM_PTR(const uint8_t, G32(0x101c4c4c)), pos[0], pos[1], font, 0x14);
        pos[1] += height + half;
        Sim_100ccc60(panel, pos, (uint32_t)(uintptr_t)font, 1);
        pos[1] += half;
        Sim_100ccc60(panel, pos, (uint32_t)(uintptr_t)font, 2);
        pos[1] += half;
        Sim_100ccc60(panel, pos, (uint32_t)(uintptr_t)font, 4);
        Sim_100ccc60(panel, pos, (uint32_t)(uintptr_t)font, 0);
        pos[1] += half;
        Sim_100ccc60(panel, pos, (uint32_t)(uintptr_t)font, 8);
        pos[0] = S32(U32(panel, 0xf0), 0);
        pos[1] += height + half;
        switch (status[0x38]) {
        case 0:
#ifdef W23F_MUTATE /* the checker's negative check only */
            if (S32(status, 0xc) >= 0)
#else
            if (S32(status, 0xc) > 0)
#endif
                W23H_SPRINTF(line, SIM_PTR(const char, 0x10151df0), SIM_PTR(const char, G32(0x101c4978)),
                             Sim_100cca20((S32(status, 0xc) + S32(status, 4)) * 0xb5 - GS32(0x1015dc78)));
            else
                W23H_SPRINTF(line, SIM_PTR(const char, 0x10151df8), SIM_PTR(const char, G32(0x101c4c38)),
                             Sim_100cca20(GS32(0x1015dc78) - S32(status, 4) * 0xb5));
            break;
        case 2:
            W23H_SPRINTF(line, SIM_PTR(const char, 0x10151e00), SIM_PTR(const char, G32(0x101c4c3c)),
                         Sim_100ccaa0(S32(status, 8)));
            break;
        case 3:
            W23H_SPRINTF(line, SIM_PTR(const char, 0x10151e10), SIM_PTR(const char, G32(0x101c4bf4)),
                         Sim_100ccaa0(S32(status, 8)));
            break;
        case 4:
            W23H_SPRINTF(line, SIM_PTR(const char, 0x10151e08), SIM_PTR(const char, G32(0x101c4bf0)),
                         Sim_100ccaa0(S32(status, 8)));
            break;
        }
        *GBYTES(0x109b992e) = 0x14;
        W23h_Text(U32(panel, 0xec), pos[0], pos[1], font, line);
        *GBYTES(0x109b992e) = 9;
    } else {
        for (i = 0; i < GS32(0x1015dc64); i++) {
            unit = BYTES(PTRTAB(i));
            if (unit == 0)
                break;
            if ((S16(unit, 0x1c) & 0x4800) != 0)
                continue;
            W23H_SPRINTF(line, SIM_PTR(const char, 0x10151e18), unit + 0x124,
                         (int)g_w34Rating[i]);
            pos[1] += height;
            *GBYTES(0x109b992e) = 0xc;
            W23h_Text(U32(panel, 0xec), pos[0], pos[1], font, line);
            *GBYTES(0x109b992e) = 9;
        }
    }
    W23H_RELEASE(G32(0x109b9900), G32(0x1014cec4));
}
