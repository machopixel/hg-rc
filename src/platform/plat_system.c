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
