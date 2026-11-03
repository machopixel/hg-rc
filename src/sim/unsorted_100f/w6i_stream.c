#include "w6i.h"
#include "../simmem.h"
#include "../util/access.h"
#include <stdarg.h>
#include "../../platform/plat.h"
#include "w8A.h"
#include "../unsorted_1010/w7r.h"
#include "../crt/hg_format.h"


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
    char *text;
    uint32_t size = 0, cap = 0x1000, got;
    int result;

    /* C runtime replacement, formatting: the rest of the stream (read with fread, in the stream's mode) scanned by
     * SDL_vsscanf. The stream is read to its end, not only as far as the format needs (DECISIONS.md). */
    text = (char *)Heap_Malloc(cap + 1);
    if (text == NULL)
        return -1;
    while ((got = Crt_FRead(text + size, 1, cap - size, (uint8_t *)(uintptr_t)a)) != 0) {
        size += got;
        if (size == cap) {
            char *more = (char *)Heap_Realloc(text, cap * 2 + 1);

            if (more == NULL)
                break;
            text = more;
            cap *= 2;
        }
    }
    text[size] = 0;
    va_start(rest, b);
    result = size == 0 ? -1 : hg_vscan(text, (const char *)(uintptr_t)b, rest);
    va_end(rest);
    Heap_FreeBlock((uint32_t)(uintptr_t)text);
    return result;
}

// FUNCTION: HGSIM 0x100fe6c0
int __cdecl File_FindClose(uint32_t handle)
{
    if (Plat_FileFindClose(handle) != 0)     /* C runtime replacement, files */
        return 0;
    G32(0x1017e888) = 0x16;
    return -1;
}

// FUNCTION: HGSIM 0x100fe6e0
int __cdecl Time_FromFileTime(const uint32_t *fileTime)
{
    uint16_t st[8]; /* SYSTEMTIME: year, month, weekday, day, hour, minute, second, milliseconds */

    if (fileTime[0] == 0 && fileTime[1] == 0)
        return -1;
    /* the original's FileTimeToLocalFileTime and FileTimeToSystemTime: the platform's (DECISIONS.md, "The last
       original code", step 3) */
    if (Plat_FileTimeToLocal(fileTime, st) == 0)
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
