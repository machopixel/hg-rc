#include <stdarg.h>
#include <string.h>
#include "w8z.h"
#include "w6i.h"
#include "w7q.h"
#include "../unsorted_1010/w6j.h"
#include "../unsorted_1010/w7s.h"

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
    struct W8zStrStream str;
    va_list args;
    int written;

    str.base = buf;
    str.ptr = buf;
    str.flag = 0x42;
    str.cnt = 0x7fffffff;
    va_start(args, format);
    written = W8Z_CALL_10100fd0(&str, format, (void *)args);
    va_end(args);
    str.cnt--;
    if (str.cnt < 0)
        W8Z_CALL_100ff030(0, &str);
    else
        *str.ptr = 0;
    return written;
}

// FUNCTION: HGSIM 0x100fcb70
double __cdecl Crt_Fabs(double x)
{
    union {
        double d;
        uint32_t w[2];
    } u;
    uint32_t saved = W8Z_CALL_10103ad0(0x133f, 0xffff);

    u.d = x;
    if ((u.w[1] & 0x7ff00000u) == 0x7ff00000u) {
        switch (Sim_10103940(x)) {
        case 1:
            W8Z_CALL_10103ad0(saved, 0xffff);
            return x;
        case 2:
            W8Z_CALL_10103ad0(saved, 0xffff);
            return -x;
        case 3:
            return W8Z_CALL_10102e30(0x15, x, saved);
        default:
            return W8Z_CALL_10102f00(8, 0x15, x, x - W8Z_GD(0x1012bdd8), saved);
        }
    }
    u.w[1] &= 0x7fffffffu;
    W8Z_CALL_10103ad0(saved, 0xffff);
    return u.d;
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
