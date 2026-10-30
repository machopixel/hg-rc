/* Debug output used by the launcher. Disabled in the retail build. */
#ifndef HG_LAUNCHER_DEBUG_H
#define HG_LAUNCHER_DEBUG_H

#include <windows.h>

enum {
    DEBUG_OFF = 0,
    DEBUG_MONO = 1,    /* DOS monochrome text screen at 0xB0000 */
    DEBUG_ODS = 2,     /* OutputDebugStringA */
    DEBUG_MSGBOX = 3,  /* MessageBoxA on g_debugHwnd */
    DEBUG_LOGFILE = 4  /* append to debug.log */
};

extern int g_debugMode;
extern HWND g_debugHwnd;

void DebugPrintf(const char *fmt, ...);

#endif
