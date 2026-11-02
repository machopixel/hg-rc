/* Link targets for platform/plat.h in the checker's flat test image; the round-28 specs hook every one of them by
 * symbol, so the bodies never run. */
#include "../../src/platform/plat.h"

void Plat_WindowShow(void *window, int how) { (void)window, (void)how; }
void Plat_WindowPainted(void *window) { (void)window; }
int32_t Plat_WindowDefault(void *window, uint32_t message, uint32_t wParam, uint32_t lParam)
{
    (void)window, (void)message, (void)wParam, (void)lParam;
    return 0;
}
uint32_t Plat_EventTime(void) { return 0; }
void Plat_RequestQuit(void) {}
void Plat_CursorVisible(int visible) { (void)visible; }
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
