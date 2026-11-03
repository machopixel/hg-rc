#include "w9e.h"
#include "../util/access.h"

static void W9e_Row(uint32_t surface, int32_t x, int32_t y, int32_t x2, int32_t shade)
{
    W9E_LINE16(surface, x, y, x2, y, 0, U16(GBYTES(0x101659e0), (uint32_t)shade * 2));
}

// FUNCTION: HGSIM 0x10051c90
void __cdecl Hud_DrawShadedBox(uint32_t surface, int32_t x, int32_t y, int32_t width, int32_t height,
                               int32_t colour)
{
    int32_t half = height / 2;
    int32_t quarter = half / 2;
    int32_t x2 = x + width;
    int32_t dark = colour - 1;
    int32_t darker = colour - 2;
    int32_t i, end;

    if (width <= 0)
        return;
    for (i = 0; i < quarter; i++)
        W9e_Row(surface, x, y + i, x2, dark);
    for (i = quarter; i < half; i++)
        W9e_Row(surface, x, y + i, x2, colour);
    end = half + (height - half) / 2;
    for (i = half; i < end; i++)
        W9e_Row(surface, x, y + i, x2, dark);
    for (i = end; i < height; i++)
        W9e_Row(surface, x, y + i, x2, darker);
}
