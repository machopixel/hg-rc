#include "w9d.h"
#include "../unsorted_100f/w2e.h"
#include "../unsorted_100f/w8z.h"
#include "../util/access.h"

// FUNCTION: HGSIM 0x10047569
char *__cdecl Sim_10047569(const char *name)
{
    char *out = SIM_PTR(char, 0x1015ef50);
    int i;

    for (i = 0; i < 0x50; i++)
        out[i] = 0;
    if ((int8_t)U8(GBYTES(0x1015f050), 0) != 0)
        Crt_SPrintf(out, SIM_PTR(char, 0x1013b2f0), SIM_PTR(char, 0x1015f050), name);
    else
        Str_Copy(out, name);
    return out;
}
