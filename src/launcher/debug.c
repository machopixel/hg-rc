#include "debug.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

/* Nothing in HG.EXE ever sets these, so all output is off in the retail game. */
// GLOBAL: HG 0x40e020
int g_debugMode = DEBUG_OFF;
// GLOBAL: HG 0x40e024
HWND g_debugHwnd = NULL;
// GLOBAL: HG 0x40b030
UINT g_debugBoxType = MB_ICONINFORMATION;
// GLOBAL: HG 0x40e028
static FILE *g_debugLog = NULL;

/*
 * Leftover from the DOS version: text written straight into the monochrome
 * adapter's video memory, 80x25 cells of (char, attribute). Under Win32 this
 * address is not mapped, so selecting DEBUG_MONO crashes, as it did in 1997.
 */
#define MONO_VRAM ((unsigned char *)0xB0000)
#define MONO_COLS 80
#define MONO_ROW_BYTES (MONO_COLS * 2)
#define MONO_LAST_ROW (MONO_VRAM + 24 * MONO_ROW_BYTES)

// FUNCTION: HG 0x401000
static void MonoScroll(void)
{
    memmove(MONO_VRAM, MONO_VRAM + MONO_ROW_BYTES, 24 * MONO_ROW_BYTES);
}

// FUNCTION: HG 0x401020
static void MonoClearLastRow(void)
{
    unsigned long *cell;

    /* Keep the attribute bytes, zero the characters. */
    for (cell = (unsigned long *)MONO_LAST_ROW;
         cell < (unsigned long *)(MONO_LAST_ROW + MONO_ROW_BYTES); cell++)
        *cell &= 0xff00ff00;
}

// FUNCTION: HG 0x401040
static void MonoPutLine(const char *text)
{
    unsigned char *dst = MONO_LAST_ROW;
    int len = (int)strlen(text);

    if (len > MONO_COLS)
        len = MONO_COLS;
    MonoScroll();
    MonoClearLastRow();
    while (len-- > 0) {
        *dst = (unsigned char)*text++;
        dst += 2;
    }
}

// FUNCTION: HG 0x401090
static void MonoPrint(const char *text)
{
    int len = (int)strlen(text);
    int i;

    for (i = 0; i < len; i += MONO_COLS)
        MonoPutLine(text + i);
}

// FUNCTION: HG 0x4010d0
static void DebugLogWrite(const char *text)
{
    if (g_debugLog == NULL) {
        g_debugLog = fopen("debug.log", "at");
        if (g_debugLog == NULL)
            return;
    }
    fputs(text, g_debugLog);
    fflush(g_debugLog);
    fclose(g_debugLog);
    g_debugLog = NULL;
}

// FUNCTION: HG 0x401130
void DebugPrintf(const char *fmt, ...)
{
    char buf[256];
    va_list args;

    va_start(args, fmt);
    _vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    switch (g_debugMode) {
    case DEBUG_MONO:
        MonoPrint(buf);
        break;
    case DEBUG_ODS:
        OutputDebugStringA(buf);
        break;
    case DEBUG_MSGBOX:
        if (g_debugHwnd != NULL)
            MessageBoxA(g_debugHwnd, buf, "DEBUG Message", g_debugBoxType);
        break;
    case DEBUG_LOGFILE:
        DebugLogWrite(buf);
        break;
    }
}
