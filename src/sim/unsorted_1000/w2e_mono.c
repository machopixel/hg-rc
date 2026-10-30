#include "w2e.h"
#include "../unsorted_1003/w2e.h"
#include "../unsorted_100f/w2e.h"
#include "../util/access.h"

#define MONO_ENABLED G32(0x10a3ade8)
#define SPACES_LINE ((char *)GBYTES(0x10a3ae20))

// FUNCTION: HGSIM 0x10007a8e
void __cdecl Mono_Enable(void)
{
    int i;

    MONO_ENABLED = 1;
    Mono_Clear();
    G32(0x1015f1e8) = 1;
    for (i = 0; i < 0x50; i++)
        SPACES_LINE[i] = 0x20;
    SPACES_LINE[0x50] = 0;
}

// FUNCTION: HGSIM 0x10008317
void __cdecl Mono_TickSpinner(void)
{
    G32(0x1012dca4) = G32(0x1012dca4) - 1;
    if (G32(0x1012dca4) == 0) {
        char glyph[2];
        uint32_t index;

        G32(0x1012dca4) = 4;
        glyph[0] = (char)GBYTES(0x1012dca0)[G32(0x1012dca8)];
        glyph[1] = 0;
        if (MONO_ENABLED != 0)
            Mono_PutText(glyph, 0x4f, 0, 0x78);
        index = G32(0x1012dca8);
        G32(0x1012dca8) = index - 1;
        if (index == 0)
            G32(0x1012dca8) = 3;
    }
}

// FUNCTION: HGSIM 0x10008390
void __cdecl Mono_PutTopLine(const char *s)
{
    char line[0x50];
    int i;

    for (i = 0; i < 0x50; i++)
        line[i] = 0;
    line[0] = 0x20;
    Str_CopyN(line + 1, s, 0x4e);
    for (i = 0; i < 0x4f; i++) {
        if (line[i] == 0)
            line[i] = 0x20;
    }
    if (MONO_ENABLED != 0)
        Mono_PutText(line, 0, 0, 0x78);
}

// FUNCTION: HGSIM 0x10008421
void __cdecl Mono_ClearLog(void)
{
    int row;

    for (row = 3; row <= 0x17; row++) {
        if (MONO_ENABLED != 0)
            Mono_PutText(SPACES_LINE, 0, (short)row, 7);
    }
    GBYTES(0x1015d024)[0] = 0;
    G32(0x1012dc7c) = 3;
    G32(0x1012dc78) = 3;
}

// FUNCTION: HGSIM 0x1000847d
void __cdecl Mono_PrintText(const char *s)
{
    int newline = 0;
    int len = (int)Str_Length(s);
    int i;

    for (i = 0; i < len; i++) {
        if (s[i] == '\n') {
            newline = 1;
        } else {
            char ch[2];

            ch[0] = s[i];
            ch[1] = 0;
            if (MONO_ENABLED != 0)
                Mono_PutText(ch, (short)G32(0x1015d080), (short)G32(0x1012dcac), 7);
        }
        G32(0x1015d080) = G32(0x1015d080) + 1;
        if (GS32(0x1015d080) > 0x4f || newline) {
            G32(0x1015d080) = 0;
            G32(0x1012dcac) = G32(0x1012dcac) + 1;
            if (GS32(0x1012dcac) > 0x17) {
                Mono_ClearLog();
                G32(0x1012dcac) = 3;
            }
            newline = 0;
        }
    }
    G32(0x1012dc78) = G32(0x1012dcac);
}
