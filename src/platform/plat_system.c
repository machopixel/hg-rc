/*
 * System services of the platform layer (plat.h) in standard C and SDL3, the same on every system: a memory block
 * that knows its size (GlobalAlloc / GlobalSize / GlobalFree), the computer's memory (GlobalMemoryStatus) and the
 * time in UTC (GetSystemTime) and a file time as local time (FileTimeToLocalFileTime + FileTimeToSystemTime), in the
 * shapes the sim used them (DECISIONS.md, "The last original code", steps 2 and 3).
 */
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <SDL3/SDL.h>

#include "plat.h"

/* the block's size sits just before it */
typedef union {
    uint32_t size;
    double align;
} PlatBlockHead;

void *Plat_BlockAlloc(uint32_t size)
{
    PlatBlockHead *h = (PlatBlockHead *)malloc(sizeof(PlatBlockHead) + size);

    if (h == NULL)
        return NULL;
    h->size = size;
    return h + 1;
}

uint32_t Plat_BlockSize(const void *block)
{
    return block != NULL ? ((const PlatBlockHead *)block - 1)->size : 0;
}

void Plat_BlockFree(void *block)
{
    if (block != NULL)
        free((PlatBlockHead *)block - 1);
}

void Plat_MemoryStatus(void *status)
{
    uint8_t *s = (uint8_t *)status;
    uint64_t bytes = (uint64_t)SDL_GetSystemRAM() * 1024u * 1024u;
    uint32_t ram = bytes > 0xffffffffu ? 0xffffffffu : (uint32_t)bytes;    /* as Windows caps it */
    uint32_t v[7];

    /* MEMORYSTATUS after its length: load 0, the RAM as total and free physical memory and page file, and a
       32-bit program's 2 GB address space; the game reads only the total physical memory (+8) */
    v[0] = 0;
    v[1] = ram;
    v[2] = ram;
    v[3] = ram;
    v[4] = ram;
    v[5] = 0x7ffe0000u;
    v[6] = 0x7ffe0000u;
    memcpy(s + 4, v, sizeof(v));
}

void Plat_SystemTime(void *systemTime)
{
    SDL_Time now = 0;
    SDL_DateTime dt;
    uint16_t v[8];

    memset(v, 0, sizeof(v));
    if (SDL_GetCurrentTime(&now) && SDL_TimeToDateTime(now, &dt, false)) {
        /* SYSTEMTIME: year, month, day of the week, day, hour, minute, second, millisecond (UTC) */
        v[0] = (uint16_t)dt.year;
        v[1] = (uint16_t)dt.month;
        v[2] = (uint16_t)dt.day_of_week;
        v[3] = (uint16_t)dt.day;
        v[4] = (uint16_t)dt.hour;
        v[5] = (uint16_t)dt.minute;
        v[6] = (uint16_t)dt.second;
        v[7] = (uint16_t)(dt.nanosecond / 1000000);
    }
    memcpy(systemTime, v, sizeof(v));
}

int Plat_FileTimeToLocal(const uint32_t *fileTime, void *systemTime)
{
    uint64_t ft = (uint64_t)fileTime[1] << 32 | fileTime[0];
    uint16_t v[8];
    time_t t;
    struct tm *tm;

    /* 11644473600 s from 1601 to 1970; the system's own local-time rules (Windows' conversion used the current
       time-zone bias for every date) */
    if (ft < 116444736000000000ull)
        return 0;
    t = (time_t)((ft - 116444736000000000ull) / 10000000u);
    tm = localtime(&t);
    if (tm == NULL)
        return 0;
    v[0] = (uint16_t)(tm->tm_year + 1900);
    v[1] = (uint16_t)(tm->tm_mon + 1);
    v[2] = (uint16_t)tm->tm_wday;
    v[3] = (uint16_t)tm->tm_mday;
    v[4] = (uint16_t)tm->tm_hour;
    v[5] = (uint16_t)tm->tm_min;
    v[6] = (uint16_t)tm->tm_sec;
    v[7] = (uint16_t)((ft / 10000u) % 1000u);
    memcpy(systemTime, v, sizeof(v));
    return 1;
}

/* ---- clock, debug text, folders, libraries (round P1: were Windows calls in plat_win32.c) ------------------ */

uint32_t Plat_TicksMs(void)
{
    /* SDL_GetTicks: ms since SDL started its clock, in 1 ms steps (GetTickCount: since Windows started, about
       15.6 ms steps); the game only takes differences, the 32 bits wrap as GetTickCount's did */
    return (uint32_t)SDL_GetTicks();
}

void Plat_DebugText(const char *text)
{
    size_t n = strlen(text);

    /* SDL's log ends every message with its own line end (on Windows it also goes to the debugger) */
    if (n > 0 && text[n - 1] == '\n')
        n--;
    if (n > 0 && text[n - 1] == '\r')
        n--;
    SDL_Log("%.*s", (int)n, text);
}

uint32_t Plat_CurrentDir(char *out, uint32_t size)
{
    char *dir = SDL_GetCurrentDirectory();
    size_t n;
    const char *prefix = "";
    size_t p;

    if (dir == NULL)
        return 0;
    n = strlen(dir);
    if (n > 1 && (dir[n - 1] == '/' || dir[n - 1] == '\\'))
        n--;                            /* GetCurrentDirectoryA gives no separator at the end */
#ifdef _WIN32
    if (n == 2 && dir[1] == ':')
        n = 3;                          /* "C:\": a drive's root keeps it */
#else
    prefix = "C:";                      /* the game takes the first character as a drive letter (w28_anim) */
#endif
    p = strlen(prefix);
    if (p + n + 1 > size) {
        SDL_free(dir);
        return (uint32_t)(p + n + 1);   /* too small: the size needed, as GetCurrentDirectoryA */
    }
    memcpy(out, prefix, p);
    memcpy(out + p, dir, n);
    out[p + n] = 0;
    SDL_free(dir);
    return (uint32_t)(p + n);
}

uint32_t Plat_ProgramPath(char *out, uint32_t size)
{
    /* the program's folder and "HG" (DECISIONS.md, round P1; only the assert box shows it) */
    const char *base = SDL_GetBasePath();
    size_t n;

    if (base == NULL)
        return 0;
    n = strlen(base);
    if (n + 3 > size)
        return 0;
    memcpy(out, base, n);
    memcpy(out + n, "HG", 3);
    return (uint32_t)(n + 2);
}

/* the libraries built into the program (the Vulkan renderer); a handle of ours is the address of its entry */
typedef struct {
    const char *name;
    void *(*function)(const char *name);
} PlatBuiltin;

static PlatBuiltin g_builtins[4];
static int g_builtinCount;

void Plat_AddBuiltinLibrary(const char *name, void *(*function)(const char *name))
{
    if (g_builtinCount < (int)(sizeof(g_builtins) / sizeof(g_builtins[0]))) {
        g_builtins[g_builtinCount].name = name;
        g_builtins[g_builtinCount].function = function;
        g_builtinCount++;
    }
}

static PlatBuiltin *Builtin(const void *handle)
{
    int i;

    for (i = 0; i < g_builtinCount; i++) {
        if (handle == &g_builtins[i])
            return &g_builtins[i];
    }
    return NULL;
}

void *Plat_LoadLibrary(const char *name)
{
    const char *file = name;
    const char *p;
    int i;

    for (p = name; *p != '\0'; p++) {
        if (*p == '\\' || *p == '/' || *p == ':')
            file = p + 1;
    }
    for (i = 0; i < g_builtinCount; i++) {
        if (SDL_strcasecmp(file, g_builtins[i].name) == 0)
            return &g_builtins[i];
    }
    return (void *)SDL_LoadObject(name);
}

void *Plat_LibraryFunction(void *library, const char *name)
{
    PlatBuiltin *b = Builtin(library);

    if (b != NULL)
        return b->function(name);
    return (void *)SDL_LoadFunction((SDL_SharedObject *)library, name);
}

void Plat_FreeLibrary(void *library)
{
    if (library != NULL && Builtin(library) == NULL)
        SDL_UnloadObject((SDL_SharedObject *)library);
}
