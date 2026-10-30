#include "w9c.h"
#include "w6e.h"
#include "w8u.h"
#include "../unsorted_100f/w2e.h"
#include "../unsorted_100f/w2g.h"
#include "../unsorted_100f/w6h.h"
#include "../unsorted_100f/w6i.h"
#include "../unsorted_100f/w8z.h"
#include "../util/access.h"

// FUNCTION: HGSIM 0x100363af
uint32_t __cdecl Plugin_ProbeList(const char *name, uint32_t arg)
{
    char line[0x100];
    void *f;

    G32(0x1015e56c) = 0;
    f = Sim_100fd850(name, SIM_PTR(const char, 0x1013a0d4));                  /* "r" */
    if (f == 0)
        return Plugin_ProbeAll(arg);
    Mem_Fill(line, 0, 0x100);
    while (Stream_ReadLine(line, 0x100, (uint8_t *)f) != 0)
        Plugin_ProbeRaster(Str_NextToken_100fd560(line, SIM_PTR(const char, 0x1013a0d8)), arg);   /* " \t\r\n" */
    Crt_FClose(f);
    return G32(0x1015e56c);
}
