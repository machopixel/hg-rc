#include <stdarg.h>
#include <string.h>
#include "w8z.h"
#include "w6i.h"
#include "w7q.h"
#include "../unsorted_1010/w6j.h"
#include "../unsorted_1010/w7s.h"
#include "../crt/hg_format.h"

/* The string stream record of sprintf; only the first four fields are set, like the original. */
struct W8zStrStream {
    char *ptr;
    int32_t cnt;
    char *base;
    int32_t flag;
    int32_t file;
    int32_t charbuf;
    int32_t bufsiz;
    char *tmpfname;
};

// FUNCTION: HGSIM 0x100fc780
int __cdecl Crt_SPrintf(char *buf, const char *format, ...)
{
    va_list args;
    int written;

    /* C runtime replacement, formatting: SDL3's formatter (crt/hg_format.h), unbounded as the original */
    va_start(args, format);
    written = hg_vformat(buf, 0x7fffffff, format, args);
    va_end(args);
    return written;
}

// FUNCTION: HGSIM 0x100fd760
int __cdecl Crt_FClose(void *stream)
{
    int result = -1;
    uint32_t flags = U32(stream, 0xc);

    if ((flags & 0x40) != 0) {
        U32(stream, 0xc) = 0;
        return -1;
    }
    if ((flags & 0x83) != 0) {
        result = Crt_FlushStream(stream);
        Sim_10104aa0(stream);
        if (W8Z_CALL_101049c0(S32(stream, 0x10)) < 0) {
            U32(stream, 0xc) = 0;
            return -1;
        }
        if (U32(stream, 0x1c) != 0) {
            Heap_FreeBlock(U32(stream, 0x1c));
            U32(stream, 0x1c) = 0;
        }
    }
    U32(stream, 0xc) = 0;
    return result;
}

// FUNCTION: HGSIM 0x100fd820
uint8_t *__cdecl Crt_FSOpen(const char *name, const char *mode, uint32_t share)
{
    uint8_t *stream = Sim_10104db0();

    if (stream == 0)
        return 0;
    return Sim_10104be0(name, mode, share, stream);
}
