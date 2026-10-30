#include "w9f.h"
#include "w8u.h"
#include "../util/access.h"
#include "../unsorted_100f/w2e.h"
#include "../unsorted_100f/w2g.h"
#include "../unsorted_100f/w6h.h"
#include "../unsorted_100f/w6i.h"
#include "../unsorted_100f/w8z.h"

// FUNCTION: HGSIM 0x10036876
uint32_t __cdecl Plugin_LoadList(const char *name)
{
    char line[0x100];
    char path[0x100];
    void *f = Sim_100fd850(name, (const char *)GBYTES(0x1013a1fc));   /* "r" */

    if (f == 0)
        return Plugin_LoadAll();
    Mem_Fill(line, 0, 0x100);
    while (Stream_ReadLine(line, 0x100, (uint8_t *)f) != 0) {
        char *token = Str_NextToken_100fd560(line, (const char *)GBYTES(0x1013a200));    /* " \t\r\n" */

        Crt_SPrintf(path, (const char *)GBYTES(0x1013a208), token);                    /* "plugin\%s" */
        W9F_CALL_1003650d(path);
    }
    Crt_FClose(f);
    return (uint32_t)(uintptr_t)GBYTES(0x1015e578);
}
