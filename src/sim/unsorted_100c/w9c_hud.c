#include "w9c.h"
#include "w7q.h"
#include "../unsorted_100f/w2e.h"
#include "../unsorted_100f/w2j.h"
#include "../unsorted_100f/w8z.h"
#include "../util/access.h"

typedef int32_t(__cdecl *W9cCFontFn)(uint32_t font);
typedef int(__cdecl *W9cCTextFn)(uint32_t pane, int32_t x, int32_t y, uint32_t font, const char *text, void *pal);

#define W9CC_CALL_100ecce0 SIM_FN(W9cCFontFn, 0x100ecce0)
#define W9CC_CALL_100f2f01 SIM_FN(W9cCTextFn, 0x100f2f01)
#define W9CC_CALL_100f1e10 SIM_FN(W9cCTextFn, 0x100f1e10)
#define W9CC_CALL_100ecea6 SIM_FN(W9cCTextFn, 0x100ecea6)

#define W9CC_ENTRY(i) (GBYTES(0x1047383a) + (uint32_t)GS32(0x1013bb40) * 0x4fc3au + (uint32_t)(i) * 0x13fu)

static void W9cC_Text(uint32_t pane, int32_t x, int32_t y, uint32_t font, const char *text)
{
    if (G32(0x101668e8) != 0)
        W9CC_CALL_100f2f01(pane, x, y, font, text, GBYTES(0x109b9920));
    else
        W9CC_CALL_100f1e10(pane, x, y, font, text, GBYTES(0x109b9920));
}

// FUNCTION: HGSIM 0x100ccc60
void __cdecl Sim_100ccc60(void *panel, int32_t *pos, uint32_t font, uint32_t kind)
{
    char text[0x100];
    uint8_t *pane = BYTES(U32(panel, 0xec));
    uint8_t *table = GBYTES(0x10473800) + (uint32_t)GS32(0x1013bb40) * 0x4fc3au;
    int32_t lineHeight = W9CC_CALL_100ecce0(font);
    int single = 0;
    int32_t i;

    /* Deviation (DECISIONS.md round 21): the last-space search of a long objective runs past its 0x20 + 1 copied
       bytes, into whatever the stack held before this call; the buffer starts zeroed instead. */
    Mem_Fill(text, 0, sizeof(text));

    if (G32(0x1015ec58) != 0 && U8(G32(0x1017b8c4), 0xa) == 0) {
        int32_t inUse = 0;
        int anyOfKind2 = 0;

        for (i = 0; i < S32(table, 0); i++) {
            uint8_t *e = W9CC_ENTRY(i);

            inUse += U8(e, 0x74);
            if (U8(e, 0x74) != 0 && U8(e, 1) == 2)
                anyOfKind2 = 1;
        }
        if (anyOfKind2 && inUse == 1)
            single = 1;
    }
    for (i = 0; i < S32(table, 0); i++) {
        uint8_t *e = W9CC_ENTRY(i);
        const char *label;
        const char *mark;
        uint8_t colour;
        int twoLines = 0;

        if (U8(e, 0x74) == 0 || U8(e, 1) != (kind & 0xff))
            continue;
        if (U32(e, 2) == 0x8000 || U32(e, 2) == 0x8001 || U32(e, 2) == 0x8002)
            continue;
        if (U8(e, 1) == 0)
            continue;
        pos[0] = S32(U32(panel, 0xf0), 0);
        switch (kind & 0xff) {
        case 1:
            label = (const char *)(uintptr_t)G32(0x101c4994);
            break;
        case 2:
            label = (const char *)(uintptr_t)(single ? G32(0x101c4994) : G32(0x101c498c));
            break;
        case 8:
            label = (const char *)(uintptr_t)G32(0x101c49a8);
            break;
        default:
            label = (const char *)(uintptr_t)G32(0x101c4988);
            break;
        }
        *GBYTES(0x109b992e) = 0x14;
        W9cC_Text(U32(panel, 0xec), pos[0], pos[1], font, label);
        *GBYTES(0x109b992e) = 9;
        pos[0] += Sim_100ccb0d((const char *)(uintptr_t)G32(0x101c49a4), (void *)(uintptr_t)font);
        if (Str_Length((const char *)e + 0xad) < 0x20) {
            *GBYTES(0x109b992e) = 0xc;
            W9cC_Text(U32(panel, 0xec), pos[0], pos[1], font, (const char *)e + 0xad);
            *GBYTES(0x109b992e) = 9;
        } else {
            int32_t x0 = pos[0];
            int32_t y0 = pos[1];
            int32_t cut = 0x20;

            Str_CopyN(text, (const char *)e + 0xad, (uint32_t)(cut + 1));
            if ((int8_t)U8(e, 0xae + cut) != ' ')
                cut = (int32_t)((uint32_t)(uintptr_t)Str_FindLast(text, ' ') - (uint32_t)(uintptr_t)text);
            Str_CopyN(text, (const char *)e + 0xad, (uint32_t)(cut + 1));
            text[cut + 1] = 0;
            *GBYTES(0x109b992e) = 0xc;
            W9cC_Text(U32(panel, 0xec), pos[0], pos[1], font, text);
            *GBYTES(0x109b992e) = 9;
            pos[0] = x0;
            pos[1] += lineHeight;
            Str_Copy(text, (const char *)e + 0xae + cut);
            *GBYTES(0x109b992e) = 0xc;
            W9cC_Text(U32(panel, 0xec), pos[0], pos[1], font, text);
            *GBYTES(0x109b992e) = 9;
            pos[0] = x0;
            pos[1] = y0;
            twoLines = 1;
        }
        if (U8(e, 0) == 5) {
            mark = (const char *)(uintptr_t)G32(0x101c49a0);
            colour = 0x17;
        } else if (U8(e, 0) == 6) {
            mark = (const char *)(uintptr_t)G32(0x101c499c);
            colour = 7;
        } else {
            mark = (const char *)(uintptr_t)G32(0x101c4980);
            colour = 0xc;
        }
        Crt_SPrintf(text, mark);
        pos[0] = (S32(pane, 0xc) - S32(pane, 4)) - Sim_100ccb0d(text, (void *)(uintptr_t)font);
        *GBYTES(0x109b992e) = colour;
        W9cC_Text(U32(panel, 0xec), pos[0], pos[1], font, text);
        *GBYTES(0x109b992e) = 9;
        pos[1] += lineHeight;
        if (twoLines)
            pos[1] += lineHeight;
    }
}
