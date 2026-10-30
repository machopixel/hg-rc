#include "w2e.h"
#include "../unsorted_100f/w2e.h"

#define MONO_SCREEN ((uint8_t *)(uintptr_t)0xB0000)

// FUNCTION: HGSIM 0x100346ad
void __cdecl Mono_PutText(const char *s, short col, short row, short attr)
{
    uint8_t *cell = MONO_SCREEN;
    int len = (int)Str_Length(s);
    int i;

    if (col >= 0x50 || col < 0 || row >= 0x19 || row < 0 || len <= 0)
        return;
    cell += (uint32_t)(int)row * 0xA0u + (uint32_t)(int)col * 2u;
    if (len > 0x50)
        len = 0x50;
    for (i = 0; i < len; i++) {
        *cell++ = (uint8_t)s[i];
        *cell++ = (uint8_t)attr;
    }
}

// FUNCTION: HGSIM 0x1003483c
void __cdecl Mono_Clear(void)
{
    Mem_Fill(MONO_SCREEN, 0x20, 0xFA0);
}
