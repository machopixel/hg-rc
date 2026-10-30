#include "w9g.h"
#include "../util/access.h"
#include "../unsorted_100f/w2e.h"
#include "../unsorted_100f/w2g.h"
#include "../unsorted_100f/w2k.h"
#include "../unsorted_100f/w6h.h"
#include "../unsorted_100f/w6i.h"
#include "../unsorted_100f/w8z.h"

#define STR(a) ((const char *)GBYTES(a))

/* Class 0x103 (the CRT's letter class: upper, lower or 0x100) of a character, read as signed like the
   original. A digit is not a letter: the skip below walks past a value with no letter, even past the line's
   end, like the original. */
static int IsLetter(char c)
{
    if (GS32(0x1015965c) > 1)
        return Crt_IsCType((int)(int8_t)c, 0x103);
    return U16(G32(0x10159450), (int)(int8_t)c * 2) & 0x103;
}

/* The first letter after the '=' of the line's first token (the original reads through a 0 result of
   Crt_StrChr, like the C does). */
static char *Value(char *line, const char *delims)
{
    char *p = Str_NextToken_100fd560(line, delims);

    p = Crt_StrChr(p, '=');
    while (IsLetter(*p) == 0)
        p++;
    return p;
}

// FUNCTION: HGSIM 0x1005f4e0
void __cdecl Lang_ReadIni(char *dll, char *simFile)
{
    void *file = Sim_100fd850(STR(0x10144238), STR(0x10144234));
    char line[0x100];
    char *p;

    if (file == 0)
        return;
    Mem_Fill(line, 0, 0x100);       /* once: a shorter line keeps the tail of the previous one after its 0 */
    for (;;) {
        if (Stream_ReadLine(line, 0x100, (uint8_t *)file) == 0)
            break;
        if (Crt_StrStr(line, STR(0x10144244)) != 0) {
            p = Value(line, STR(0x1014424c));
            if (p != 0)
                Str_Copy(dll, p);
        } else if (Crt_StrStr(line, STR(0x10144250)) != 0) {
            p = Value(line, STR(0x1014425c));
            if (p != 0)
                Str_Copy(simFile, p);
        }
    }
    Crt_FClose(file);
}
