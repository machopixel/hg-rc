#include "w8u.h"
#include "w6e.h"
#include "w7m.h"
#include "../unsorted_100f/w6i.h"
#include "../unsorted_100f/w7q.h"
#include "../util/access.h"

/* The C library's _finddata_t: 0x118 bytes, the file name at +0x14. */
#define W8U_FIND_SIZE 0x118
#define W8U_FIND_NAME(fd) ((char *)(fd) + 0x14)

// FUNCTION: HGSIM 0x10035f90
uint32_t __cdecl Plugin_ProbeAll(uint32_t arg)
{
    uint32_t fd[W8U_FIND_SIZE / 4];
    int32_t h;

    G32(0x1015e56c) = 0;
    h = Crt_FindFirst(SIM_PTR(const char, 0x1013a068), fd);        /* "plugin\*.dll" */
    if (h == -1)
        return G32(0x1015e56c);
    Plugin_ProbeRaster(W8U_FIND_NAME(fd), arg);
    while (Crt_FindNext((uint32_t)h, fd) == 0)
        Plugin_ProbeRaster(W8U_FIND_NAME(fd), arg);
    File_FindClose((uint32_t)h);
    return G32(0x1015e56c);
}

// FUNCTION: HGSIM 0x10036108
uint32_t __cdecl Sim_10036108(uint8_t *list)
{
    uint32_t fd[W8U_FIND_SIZE / 4];
    char path[0x100];
    int32_t h;

    G32(0x1015e574) = 0;
    h = Crt_FindFirst(SIM_PTR(const char, 0x1013a068), fd);        /* "plugin\*.dll" */
    if (h == -1)
        return G32(0x1015e574);
    W8U_SPRINTF_1003(path, SIM_PTR(const char, 0x1013a09c), W8U_FIND_NAME(fd));   /* "plugin\%s" */
    Sim_100361d8(path, list);
    while (Crt_FindNext((uint32_t)h, fd) == 0) {
        W8U_SPRINTF_1003(path, SIM_PTR(const char, 0x1013a0a8), W8U_FIND_NAME(fd));
        Sim_100361d8(path, list);
    }
    File_FindClose((uint32_t)h);
    return G32(0x1015e574);
}

// FUNCTION: HGSIM 0x10036466
uint32_t __cdecl Plugin_LoadAll(void)
{
    uint32_t fd[W8U_FIND_SIZE / 4];
    char path[0x100];
    int32_t h;

    G32(0x1015e570) = 0;
    h = Crt_FindFirst(SIM_PTR(const char, 0x1013a068), fd);        /* "plugin\*.dll" */
    if (h == -1)
        return 0;
    W8U_CALL_1003650d(W8U_FIND_NAME(fd));
    while (Crt_FindNext((uint32_t)h, fd) == 0) {
        W8U_SPRINTF_1003(path, SIM_PTR(const char, 0x1013a0e0), W8U_FIND_NAME(fd));   /* "plugin\%s" */
        W8U_CALL_1003650d(path);
    }
    File_FindClose((uint32_t)h);
    return (uint32_t)(uintptr_t)GBYTES(0x1015e578);
}
