#include "w9f.h"
#include "w7n.h"
#include "../util/access.h"
#include "../unsorted_100f/w2g.h"
#include "../unsorted_100f/w6h.h"
#include "../unsorted_100f/w6i.h"
#include "../unsorted_100f/w7r.h"
#include "../unsorted_100f/w8z.h"

// FUNCTION: HGSIM 0x10054a79
int __cdecl GameKey_LoadMap(void)
{
    char line[0x100];
    char action[0x28];
    char keys[0x28];
    void *f;
    char *token;
    int16_t sum;
    uint32_t i;
    int32_t code;
    uint8_t *slot;

    GBYTES(0x10164be8)[0] = 'k';
    GBYTES(0x10164d39)[0] = 'v';
    G32(0x1015fdf8) = 0;
    f = Sim_100fd850((const char *)GBYTES(0x10141ca4), (const char *)GBYTES(0x10141ca0));   /* "gamekey.map", "r" */
    if (f == 0) {
        W9F5_CALL_100bc280(0x11, (const char *)GBYTES(0x10141cb0), 0);
        return 0;
    }
    while (Input_ReadLine(line, 0xff, (uint8_t *)f) != 0) {
        if (Sim_100fd710(line, (const char *)GBYTES(0x10141cd0), action, keys) != 2) {       /* "%s %s" */
            W9F5_CALL_100bc280(0x11, (const char *)GBYTES(0x10141cd8), G32(0x1015fdf8), 0);
            return 0;
        }
        token = Str_NextToken_100fd560(keys, (const char *)GBYTES(0x10141d04));              /* "+" */
        if (token == 0) {
            W9F5_CALL_100bc280(0x11, (const char *)GBYTES(0x10141d08), G32(0x1015fdf8), 0);
            return 0;
        }
        sum = 0;
        while (token != 0) {
            code = 0;
            for (i = 0; i < 0x1b; i++) {
                if (Str_CompareNoCase((const char *)(uintptr_t)U32(GBYTES(0x101402d8), i * 6), token) == 0) {
                    code = S16(GBYTES(0x101402dc), i * 6);
                    break;
                }
            }
            if (code != 0)
                sum = (int16_t)(sum + (int16_t)code);
            else if ((uint32_t)(int32_t)(int8_t)token[0] < 0x80u)
                sum = (int16_t)(sum + (int16_t)Crt_ToLower((int8_t)token[0]));
            else
                sum = (int16_t)(sum + (int16_t)(int8_t)token[0]);
            token = Str_NextToken_100fd560(0, (const char *)GBYTES(0x10141d38));              /* "+" */
        }
        for (i = 0; i < 0xc4; i++) {
            if (Str_CompareNoCase((const char *)(uintptr_t)U32(GBYTES(0x1013ff00), i * 5), action) == 0)
                break;
        }
        if (i >= 0xc4) {
            W9F5_CALL_100bc280(0x11, (const char *)GBYTES(0x10141d3c), G32(0x1015fdf8), action);
            return 0;
        }
        slot = GBYTES(0x10164bc8) + (int32_t)sum;
        if (slot[0] != 0 && slot[0] != U8(GBYTES(0x1013ff04), i * 5)) {
            W9F5_CALL_100bc280(0x11, (const char *)GBYTES(0x10141d6c), G32(0x1015fdf8), line);
            return 0;
        }
        slot[0] = U8(GBYTES(0x1013ff04), i * 5);
    }
    Crt_FClose(f);
    return 1;
}
