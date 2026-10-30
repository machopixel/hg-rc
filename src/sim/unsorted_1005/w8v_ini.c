#include "w8v.h"
#include "w7m.h"
#include "../util/access.h"
#include "../unsorted_100f/w2e.h"
#include "../unsorted_100f/w2j.h"
#include "../unsorted_100f/w2k.h"
#include "../unsorted_100f/w6h.h"
#include "../unsorted_100f/w6i.h"
#include "../unsorted_100f/w7q.h"

/* The file position just after the "[section]" line found last, -1 when none. */
#define SECTION_POS GS32(0x1013f10c)

/*
 * Looks for the line "[section]" in HG.INI (blanks around the name are ignored, the case too) and keeps the file
 * position after it. Returns 0, 0x41 when HG.INI cannot be opened, 0x4a when there is no such section.
 */
// FUNCTION: HGSIM 0x100531e0
int __cdecl Ini_FindSection(const char *section)
{
    char line[0x88];
    char *p, *q;
    void *f;

    SECTION_POS = -1;
    f = Sim_100fd850(SIM_PTR(char, 0x1013f114), SIM_PTR(char, 0x1013f110));
    if (f == 0)
        return 0x41;
    section = Str_TrimSpace((char *)section);
    for (;;) {
        if (Stream_ReadLine(line, 0x85, f) == 0) {
            W8V_FCLOSE5(f);
            return 0x4a;
        }
        p = line + Crt_StrSpn(line, SIM_PTR(char, 0x1013f11c));
        if (*p != '[')
            continue;
        p++;
        q = Crt_StrChr(p, ']');
        if (q == 0)
            continue;
        *q = 0;
        if (Str_CompareNoCase(Str_TrimSpace(p), section) == 0)
            break;
    }
    SECTION_POS = Crt_FTell(f);
    W8V_FCLOSE5(f);
    return 0;
}

/*
 * The value of "key = value" in the section found by Ini_FindSection (the line buffer is 0x1015f3b8), with the
 * line end removed and, unless raw, cut at ';' and trimmed. When the key is missing the text at 0x1015f444 is
 * returned (0x1015f440 when there is no section or no HG.INI).
 */
// FUNCTION: HGSIM 0x10053323
char *__cdecl Ini_GetValue(const char *key, int raw)
{
    char *const line = SIM_PTR(char, 0x1015f3b8);
    char *p, *value;
    uint32_t n;
    void *f;

    if (SECTION_POS == -1)
        return SIM_PTR(char, 0x1015f440);
    f = Sim_100fd850(SIM_PTR(char, 0x1013f124), SIM_PTR(char, 0x1013f120));
    if (f == 0)
        return SIM_PTR(char, 0x1015f440);
    W8V_FSEEK5(f, SECTION_POS, 0);
    key = Str_TrimSpace((char *)key);
    for (;;) {
        value = SIM_PTR(char, 0x1015f444);
        if (Stream_ReadLine(line, 0x85, f) == 0)
            break;
        p = line + Crt_StrSpn(line, SIM_PTR(char, 0x1013f12c));
        if (*p == '[')
            break;
        value = Crt_StrChr(p, '=');
        if (value == 0)
            continue;
        *value = 0;
        value++;
        if (Str_CompareNoCase(Str_TrimSpace(p), key) == 0)
            break;
    }
    W8V_FCLOSE5(f);
    n = Str_Length(value);
    if (n > 0)
        value[n - 1] = 0;
    if (raw == 0) {
        value[Str_CSpan(value, SIM_PTR(char, 0x1013f130))] = 0;
        value = Str_TrimSpace(value);
    }
    return value;
}
