#include <stdarg.h>

#include "w28.h"
#include "../util/access.h"
#include "../unsorted_1005/w8v.h"
#include "../unsorted_100b/w6g.h"
#include "../unsorted_100b/w7p.h"
#include "../unsorted_100f/w2e.h"
#include "../unsorted_100f/w2j.h"
#include "../unsorted_100f/w8z.h"
#include "../unsorted_1010/w8A.h"

#define W28_TEXT ((char *)GBYTES(0x1017d618))

/* What each error code 1..0x58 does (the original's switch table at 0x100bc3f9): 0 nothing more, 1 a warning
 * (Sim_100bc535), 2 the game shut down (Sim_100bc451) then the fatal box (Sim_100bc47e), 3 the fatal box. */
static const uint8_t kW28ErrorKind[0x58] = {
    1, 2, 2, 2, 2, 2, 0, 1, 2, 1, 2, 0, 2, 2, 2, 3, 3, 1, 2, 2, 2, 1, 1, 2,
    2, 2, 2, 2, 1, 1, 1, 1, 2, 2, 2, 0, 1, 2, 1, 1, 2, 2, 2, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 2, 1, 1, 1, 2, 2, 2, 2, 1, 2, 2, 2, 2, 2, 2, 3, 3, 1, 1, 2,
    2, 2, 1, 3, 2, 2, 2, 2, 2, 2, 2, 3, 2, 2, 1, 2};

void(__cdecl *g_w28ErrorTrace)(uint32_t code, const char *format);

// FUNCTION: HGSIM 0x100bc280
void __cdecl Sim_100bc280(uint32_t code, const char *format, ...)
{
    char key[12];
    va_list args;

    if (g_w28ErrorTrace != 0)
        g_w28ErrorTrace(code, format);
    Crt_SPrintf(key, (const char *)GBYTES(0x10151448), code);
    Ini_FindSection((const char *)GBYTES(0x10151450));
    Crt_SPrintf(W28_TEXT, (const char *)GBYTES(0x1015145c), code, Ini_GetValue(key, 1));
    if (format != 0) {
        va_start(args, format);
        Str_Cat(W28_TEXT, (const char *)GBYTES(0x10151468));
        Crt_VSPrintf(W28_TEXT + Str_Length(W28_TEXT), format, (void *)args);
        va_end(args);
    }
    G32(0x1017de18) = code;
    if (code - 1 >= 0x58)
        return;
    switch (kW28ErrorKind[code - 1]) {
    case 1:
        Sim_100bc535((uint32_t)(uintptr_t)W28_TEXT);
        break;
    case 2:
#ifndef W28E_MUTATE /* the checker's negative check only */
        Sim_100bc451();
#endif
        /* fall through */
    case 3:
        Sim_100bc47e((uint32_t)(uintptr_t)W28_TEXT);
        break;
    default:
        break;
    }
}
