#include "w6i.h"
#include "../simmem.h"
#include "../util/access.h"
#include <stdarg.h>

typedef int(__stdcall *W6iFindCloseFn)(uint32_t handle);
typedef int(__stdcall *W6iFileTimeFn)(const void *in, void *out);

// FUNCTION: HGSIM 0x100fe310
int __cdecl Sim_100fe310(int mode)
{
    int done = 0;
    int failed = 0;
    int i;

    for (i = 0; i < GS32(0x10ac4ba0); i++) {
        uint32_t stream = U32(G32(0x10ac3860), 4u * (uint32_t)i);
        uint8_t flags;

        if (stream == 0)
            continue;
        flags = U8(stream, 0xc);
        if ((flags & 0x83) == 0)
            continue;
        if (mode == 1) {
            if (W6I_FN_100FE240(BYTES(stream)) != -1)
                done++;
        } else if (mode == 0 && (flags & 2) != 0) {
            if (W6I_FN_100FE240(BYTES(stream)) == -1)
                failed = -1;
        }
    }
    return mode == 1 ? done : failed;
}

// FUNCTION: HGSIM 0x100fe450
int __cdecl Sim_100fe450(uint32_t a, uint32_t b, ...)
{
    va_list rest;
    int result;

    va_start(rest, b);
    result = W6I_FN_10103BA0(a, b, (void *)rest);
    va_end(rest);
    return result;
}

// FUNCTION: HGSIM 0x100fe6c0
int __cdecl File_FindClose(uint32_t handle)
{
    if (SIM_API(W6iFindCloseFn, 0x10ac55f0)(handle) != 0)
        return 0;
    G32(0x1017e888) = 0x16;
    return -1;
}

// FUNCTION: HGSIM 0x100fe6e0
int __cdecl Time_FromFileTime(const uint32_t *fileTime)
{
    uint32_t local[2];
    uint16_t st[8]; /* SYSTEMTIME: year, month, weekday, day, hour, minute, second, milliseconds */

    if (fileTime[0] == 0 && fileTime[1] == 0)
        return -1;
    if (SIM_API(W6iFileTimeFn, 0x10ac563c)(fileTime, local) == 0)
        return -1;
    if (SIM_API(W6iFileTimeFn, 0x10ac5630)(local, st) == 0)
        return -1;
    return W6I_FN_10105970(st[0], st[1], st[3], st[4], st[5], st[6], -1);
}

// FUNCTION: HGSIM 0x100fe770
char *__cdecl Stream_ReadLine(char *buf, int n, uint8_t *f)
{
    char *p = buf;

    if (n <= 0)
        return 0;
    while (--n != 0) {
        uint32_t left = U32(f, 4) - 1;
        int c;

        U32(f, 4) = left;
        if ((int32_t)left >= 0) {
            uint8_t *src = BYTES(U32(f, 0));

            c = *src;
            U32(f, 0) = (uint32_t)(uintptr_t)(src + 1);
        } else {
            c = W6I_FN_10105A60(f);
        }
        if (c == -1) {
            if (p == buf)
                return 0;
            break;
        }
        *p++ = (char)c;
        if ((uint8_t)c == '\n')
            break;
    }
    *p = 0;
    return buf;
}
