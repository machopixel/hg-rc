#include "w9b.h"
#include "../unsorted_1006/w9b.h"
#include "../unsorted_100b/w6g.h"
#include "../unsorted_100f/w8z.h"
#include "../util/access.h"

#define SELF G32(0x1017b8c8)
#define TEXT(addr) SIM_PTR(const char, addr)
#define TEXT_AT(addr) ((const char *)(uintptr_t)G32(addr))
#define PTR_AT(addr) ((void *)(uintptr_t)G32(addr))

static void *W9b_Font(void)
{
    return W9BC_CALL_100ba7d5(PTR_AT(0x109b9900), PTR_AT(0x1014cec4), 0);
}

/* The text drawn at the pane's position with the routine of the display mode. */
static void W9b_Draw(uint8_t *pane, void *font, const char *text)
{
    uint32_t y = U32(U32(pane, 0xf0), 4);
    uint32_t x = U32(U32(pane, 0xf0), 0);
    uint32_t target = U32(pane, 0xec);

    if (G32(0x101668e8) != 0)
        W9BC_CALL_100f2f01(target, x, y, font, text, SIM_PTR(void, 0x109b9920));
    else
        W9BC_CALL_100f1e10(target, x, y, font, text, SIM_PTR(void, 0x109b9920));
}

static void W9b_ReleaseFont(void)
{
    Sim_100bb2b0((const uint32_t *)(uintptr_t)G32(0x109b9900), TEXT_AT(0x1014cec4));
}

// FUNCTION: HGSIM 0x100cf69a
void __cdecl Hud_DrawScore(uint8_t *pane)
{
    void *font;
    char text[0x20];

    if (S16(pane, 4) == 0)
        return;
    font = W9b_Font();
    if (font == 0)
        return;
    if (G32(0x1015ec58) != 0 && G32(0x1015f208) == 0 && G32(0x10179cd4) == 0 && G32(0x1017e5e0) == 0) {
        Crt_SPrintf(text, TEXT(0x10151e68), TEXT_AT(0x101c4c2c), G32(0x10179cac));
        W9b_Draw(pane, font, text);
    }
    W9b_ReleaseFont();
}

// FUNCTION: HGSIM 0x100cf7e3
void __cdecl Hud_DrawStatusText(uint8_t *pane)
{
    uint8_t *block;
    const char *text;
    void *font;

    if (S16(pane, 4) == 0)
        return;
    block = BYTES(U32(PTRTAB(SELF), 0x28));
    switch (U32(block, 0x50c)) {
    case 1:
    case 2:
        text = TEXT_AT(0x101c4c48);
        break;
    case 6:
        text = TEXT(0x1017e5f8);
        FlashText_Start(3);
        U32(block, 0x50c) = 0;
        break;
    default:
        return;
    }
    font = W9b_Font();
    if (font == 0)
        return;
    *GBYTES(0x109b992e) = 0xf;
    if ((int8_t)text[0] != 0)
        W9b_Draw(pane, font, text);
    *GBYTES(0x109b992e) = 9;
    W9b_ReleaseFont();
}
