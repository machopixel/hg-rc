/* Link targets for platform/plat.h in the checker's flat test image; the round-28 specs hook every one of them by
 * symbol, so the bodies never run. */
#include "../../src/platform/plat.h"

void Plat_WindowPainted(void *window) { (void)window; }
int32_t Plat_WindowDefault(void *window, uint32_t message, uint32_t wParam, uint32_t lParam)
{
    (void)window, (void)message, (void)wParam, (void)lParam;
    return 0;
}
uint32_t Plat_EventTime(void) { return 0; }
void Plat_RequestQuit(void) {}
uint32_t Plat_TicksMs(void) { return 0; }
uint32_t Plat_CurrentDir(char *out, uint32_t size) { (void)out, (void)size; return 0; }
uint32_t Plat_IniInt(const char *path, const char *section, const char *key, uint32_t def)
{
    (void)path, (void)section, (void)key;
    return def;
}
uint32_t Plat_IniString(const char *path, const char *section, const char *key, const char *def, char *out,
                        uint32_t size)
{
    (void)path, (void)section, (void)key, (void)def, (void)out, (void)size;
    return 0;
}
void *Plat_LoadLibrary(const char *name) { (void)name; return 0; }
void *Plat_LibraryFunction(void *library, const char *name) { (void)library, (void)name; return 0; }
void Plat_FreeLibrary(void *library) { (void)library; }

/* Files (C runtime replacement, files): the stand-ins call the Windows import the original called, through the
 * DLL's import slot, so the checker logs and answers them exactly as for the original's run. */
#include "../../src/sim/simmem.h"

#define PS_API(type, slot) SIM_API(type, slot)
typedef uint32_t(__stdcall *PsCreateFn)(const char *, uint32_t, uint32_t, void *, uint32_t, uint32_t, uint32_t);
typedef int32_t(__stdcall *PsXferFn)(uint32_t, void *, uint32_t, uint32_t *, void *);
typedef uint32_t(__stdcall *PsSeekFn)(uint32_t, int32_t, int32_t *, uint32_t);
typedef int32_t(__stdcall *PsHandleFn)(uint32_t);
typedef uint32_t(__stdcall *PsTypeFn)(uint32_t);
typedef int32_t(__stdcall *PsNameFn)(const char *);
typedef uint32_t(__stdcall *PsVoidFn)(void);
typedef uint32_t(__stdcall *PsFindFirstFn)(const char *, void *);
typedef int32_t(__stdcall *PsFindNextFn)(uint32_t, void *);

uint32_t Plat_FileCreate(const char *name, uint32_t access, uint32_t share, void *security, uint32_t disposition,
                         uint32_t flags, uint32_t tmpl)
{
    return PS_API(PsCreateFn, 0x10ac56d8)(name, access, share, security, disposition, flags, tmpl);
}
int32_t Plat_FileRead(uint32_t handle, void *data, uint32_t size, uint32_t *done, void *overlapped)
{
    return PS_API(PsXferFn, 0x10ac5690)(handle, data, size, done, overlapped);
}
int32_t Plat_FileWrite(uint32_t handle, const void *data, uint32_t size, uint32_t *done, void *overlapped)
{
    return PS_API(PsXferFn, 0x10ac568c)(handle, (void *)data, size, done, overlapped);
}
uint32_t Plat_FileSeek(uint32_t handle, int32_t distance, int32_t *high, uint32_t method)
{
    return PS_API(PsSeekFn, 0x10ac56f4)(handle, distance, high, method);
}
int32_t Plat_FileTruncate(uint32_t handle) { return PS_API(PsHandleFn, 0x10ac56e8)(handle); }
int32_t Plat_FileFlush(uint32_t handle) { return PS_API(PsHandleFn, 0x10ac5688)(handle); }
int32_t Plat_FileClose(uint32_t handle) { return PS_API(PsHandleFn, 0x10ac566c)(handle); }
uint32_t Plat_FileType(uint32_t handle) { return PS_API(PsTypeFn, 0x10ac569c)(handle); }
int32_t Plat_FileDelete(const char *name) { return PS_API(PsNameFn, 0x10ac56f0)(name); }
uint32_t Plat_FileLastError(void) { return PS_API(PsVoidFn, 0x10ac5600)(); }
uint32_t Plat_FileFindFirst(const char *pattern, void *data) { return PS_API(PsFindFirstFn, 0x10ac55f8)(pattern, data); }
int32_t Plat_FileFindNext(uint32_t find, void *data) { return PS_API(PsFindNextFn, 0x10ac55f4)(find, data); }
int32_t Plat_FileFindClose(uint32_t find) { return PS_API(PsHandleFn, 0x10ac55f0)(find); }

/* The program's name and the assert box (C runtime replacement, the rest): reached only through Crt_Assert, whose
 * checker stand-in calls the original assert, so these never run in a spec. */
uint32_t Plat_ProgramPath(char *out, uint32_t size) { (void)out, (void)size; return 0; }
int Plat_AssertBox(const char *text, const char *caption) { (void)text, (void)caption; return PLAT_ASSERT_IGNORE; }
void Plat_DebugBreak(void) {}

/* Windows calls (DECISIONS.md, "Windows calls"): the stand-ins make the call the converted code made before,
 * through the DLL's import slot, so the specs answer and log it as for the original. */
typedef uint32_t(__stdcall *PsTimeFn)(void);
typedef void(__stdcall *PsTextFn)(const char *);
typedef uint32_t(__stdcall *PsMapKeyFn)(uint32_t, uint32_t);
typedef int16_t(__stdcall *PsKeyStateFn)(int);

uint32_t Plat_TimeMs(void) { return PS_API(PsTimeFn, 0x10ac5868)(); }
void Plat_DebugText(const char *text) { PS_API(PsTextFn, 0x10ac55e0)(text); }
uint32_t Plat_KeyChar(uint32_t key) { return PS_API(PsMapKeyFn, 0x10ac578c)(key, 2); }
int16_t Plat_KeyState(int key) { return PS_API(PsKeyStateFn, 0x10ac5788)(key); }

/* The screen (DECISIONS.md, "Windows calls: the screen"): where a platform call stands for the original's Windows
 * call(s), the stand-in makes them through the DLL's import slots, so the specs answer and log them as for the
 * original. The round-28 specs hook Plat_WindowShow and Plat_CursorVisible by symbol, so these bodies do not change
 * them. */
typedef int32_t(__stdcall *PsIntFn)(int32_t);
typedef int32_t(__stdcall *PsShowFn)(void *, int32_t);
typedef int32_t(__stdcall *PsWindowFn)(void *);
typedef int32_t(__stdcall *PsBoxFn)(void *, const char *, const char *, uint32_t);
typedef int32_t(__stdcall *PsSendFn)(void *, uint32_t, uint32_t, uint32_t);
typedef int32_t(__stdcall *PsPeekFn)(void *, void *, uint32_t, uint32_t, uint32_t);
typedef int32_t(__stdcall *PsCounterFn)(void *);

void Plat_WindowShow(void *window, int how)
{
    static const int32_t kShow[5] = {0, 4, 6, 9, 10};   /* SW_HIDE, SHOWNOACTIVATE, MINIMIZE, RESTORE, SHOWDEFAULT */

    PS_API(PsShowFn, 0x10ac57dc)(window, kShow[how]);
    if (how == PLAT_SHOW_DEFAULT)
        PS_API(PsWindowFn, 0x10ac57e0)(window);           /* UpdateWindow */
}
/* The original's ShowCursor loops, call for call as the spec answers them; at most 64 calls, so a spec whose
 * ShowCursor keeps answering "hidden" (spec_w21_simmain's countdown) cannot hold the loop for ever where the original
 * made one call (SimMain's last ShowCursor(TRUE)). */
void Plat_CursorVisible(int visible)
{
    int n = 0;

    if (visible) {
        while (PS_API(PsIntFn, 0x10ac5768)(1) < 0 && ++n < 64) {
        }
    } else {
        while (PS_API(PsIntFn, 0x10ac5768)(0) >= 0 && ++n < 64) {
        }
    }
}
int Plat_CursorShown(void) { return 1; }
void Plat_ScreenSize(int *width, int *height)
{
    *width = PS_API(PsIntFn, 0x10ac577c)(0);
    *height = PS_API(PsIntFn, 0x10ac577c)(1);
}
void Plat_WindowPlace(void *window, int fullscreen, int x, int y, int width, int height)
{
    (void)window, (void)fullscreen, (void)x, (void)y, (void)width, (void)height;
}
int Plat_WindowIsFullscreen(void *window) { (void)window; return 1; }
void Plat_WindowedSize(int *width, int *height) { *width = 1280, *height = 960; }
void Plat_WindowRect(void *window, int32_t *rect)
{
    (void)window;
    rect[0] = rect[1] = 0;
    rect[2] = 640, rect[3] = 480;
}
int Plat_MessageBox(void *window, const char *text, const char *caption, uint32_t type)
{
    return PS_API(PsBoxFn, 0x10ac57e4)(window, text, caption, type);
}
uint64_t Plat_CounterNow(void)
{
    uint32_t c[2] = {0, 0};

    PS_API(PsCounterFn, 0x10ac55e8)(c);                   /* QueryPerformanceCounter */
    return (uint64_t)c[1] << 32 | c[0];
}
uint64_t Plat_CounterRate(void) { return 1; }
int32_t Plat_WindowSend(void *window, uint32_t message, uint32_t wParam, uint32_t lParam)
{
    return PS_API(PsSendFn, 0x10ac57b8)(window, message, wParam, lParam);
}
int Plat_DropEvent(void)
{
    uint8_t msg[0x1c];

    return PS_API(PsPeekFn, 0x10ac5780)(msg, 0, 0, 0, 1) != 0;
}
