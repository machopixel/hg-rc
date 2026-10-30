#include "w8u.h"
#include "w2k.h"
#include "../unsorted_1000/w2e.h"
#include "../unsorted_100f/w2e.h"
#include "../unsorted_100f/w6h.h"
#include "../unsorted_100f/w7q.h"
#include "../util/access.h"

// FUNCTION: HGSIM 0x10027b19
void __cdecl Debug_LogText(const char *text)
{
    void *f = Sim_100fd850(SIM_PTR(const char, 0x10134ff0), SIM_PTR(const char, 0x10134fec));   /* "hgdebug.txt", "a" */

    if (f != 0)
        Crt_FPrintf(f, SIM_PTR(const char, 0x10134ffc), text);                              /* "%s" */
    W8U_FCLOSE_1002(f);
    Mono_PrintText(text);
}

// FUNCTION: HGSIM 0x10027b69
void __cdecl Debug_LogCodeText(uint32_t code)
{
    char text[0x104];

    W8U_SPRINTF_1002(text, SIM_PTR(const char, 0x10135000), Sim_10027acf(code));            /* "%s, " */
    Debug_LogText(text);
}

// FUNCTION: HGSIM 0x100278c0
uint8_t *__cdecl Sim_100278c0(const uint8_t *rec, uint8_t *out)
{
    char text[0x110];
    int32_t id = S16(rec, 0);
    uint8_t *res;

    Str_CopyN((char *)out + 6, (const char *)rec + 2, 8);
    out[0xe] = 0;
    if (G32(0x1015dc28) != 0) {
        if ((int8_t)rec[2] != 0)
            W8U_SPRINTF_1002(text, SIM_PTR(const char, 0x10134fb0), rec + 2);              /* "\n%s: " */
        else
            W8U_SPRINTF_1002(text, SIM_PTR(const char, 0x10134fb8), (int)S16(rec, 0));     /* "\nPrj file ID# %d: " */
        Debug_LogText(text);
    }
    U32(out, 0) = 1;
    U16(out, 4) = (uint16_t)id;
    res = W8U_CALL_100ba7d5((void *)(rec + 2), G32(0x1014cefc), 0);
    if (res == 0)
        W8U_CALL_100bc280(0x2a, SIM_PTR(const char, 0x10134fcc), rec + 2, (int)S16(rec, 0), 0);   /* "%s ID %d" */
    if (res == 0)
        return 0;
    U32(out, 0xf) = U32(res, 4);
    U32(out, 0x13) = U32(res, 8);
    U32(out, 0x17) = (uint32_t)(uintptr_t)res;
    U32(out, 0x1b) = (uint32_t)(uintptr_t)res;
    return out;
}
