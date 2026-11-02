/*
 * Round 8, letter z: C runtime code of HGSIM.DLL at 0x100fc780..0x100fd820 (string formatting, closing and
 * opening streams, fabs), and the float helpers the other w8z files share.
 *
 * Stream records are used as offsets (as in w6j.h / w7q.h): +0 next byte, +4 bytes left, +8 buffer, +0xc flags,
 * +0x10 file handle, +0x1c name of a temporary file.
 *
 * Callees that are not converted are called through their original address (SIM_FN):
 *   0x10100fd0 (stream, format, pointer to the variable arguments)   the formatting engine
 *   0x100ff030 (character, stream)                                   stores a character when the buffer is full
 *   0x101049c0 (file handle)                                         closes a file handle
 *   0x10103ad0 (new, mask) -> old                                    sets the x87 control word
 *   0x10102e30 (operation, x, saved control word)                    math error for a quiet NaN, x87 result
 *   0x10102f00 (flags, operation, x, result, saved control word)     math error, x87 result
 */
#ifndef HG_SIM_UNSORTED_100F_W8Z_H
#define HG_SIM_UNSORTED_100F_W8Z_H

#include <stdint.h>
#include "../simmem.h"
#include "../util/access.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* ---- float helpers shared by the w8z files ---- */
typedef float HG_MAY_ALIAS w8zf32;
typedef double HG_MAY_ALIAS w8zf64;

#define W8Z_FL(p, off) (*(w8zf32 *)(BYTES(p) + (off)))
#define W8Z_GF(addr) (*SIM_PTR(w8zf32, addr))
#define W8Z_GD(addr) (*SIM_PTR(w8zf64, addr))

static inline float W8z_Float(uint32_t u)
{
    union {
        float f;
        uint32_t u;
    } x;

    x.u = u;
    return x.f;
}

static inline uint32_t W8z_Bits(float f)
{
    union {
        float f;
        uint32_t u;
    } x;

    x.f = f;
    return x.u;
}

/*
 * The runtime's float-to-int truncation 0x100fcb08 (x87 FISTP of a 64-bit integer with round toward zero, the
 * low 32 bits returned; out of range values and NaN give the 64-bit "integer indefinite", low half 0). Same as
 * Render_Ftol in render/w2n.h. Exact for the values this code hands it: they are doubles (the x87 runs with
 * 53-bit precision), not wider.
 */
static inline int32_t W8z_Ftol(double d)
{
    if (!(d >= -9223372036854775808.0 && d < 9223372036854775808.0))
        return 0;
    return (int32_t)(uint32_t)(uint64_t)(int64_t)d;
}

typedef int(__cdecl *W8zOutputFn)(void *stream, const char *format, void *args);
typedef int(__cdecl *W8zFlushCharFn)(int ch, void *stream);
typedef int(__cdecl *W8zCloseFn)(int fd);
typedef uint32_t(__cdecl *W8zControlFn)(uint32_t value, uint32_t mask);
typedef double(__cdecl *W8zQnanFn)(int operation, double x, uint32_t savedcw);
typedef double(__cdecl *W8zExceptFn)(int flags, int operation, double x, double result, uint32_t savedcw);

#define W8Z_CALL_10100fd0 SIM_FN(W8zOutputFn, 0x10100fd0)
#define W8Z_CALL_100ff030 SIM_FN(W8zFlushCharFn, 0x100ff030)
#define W8Z_CALL_101049c0 SIM_FN(W8zCloseFn, 0x101049c0)
#define W8Z_CALL_10103ad0 SIM_FN(W8zControlFn, 0x10103ad0)
#define W8Z_CALL_10102e30 SIM_FN(W8zQnanFn, 0x10102e30)
#define W8Z_CALL_10102f00 SIM_FN(W8zExceptFn, 0x10102f00)

/* ---- w8z_crt.c ---- */
/*
 * sprintf: formats into buf through a string stream record on the stack (flags 0x42, 0x7fffffff bytes left)
 * with 0x10100fd0, then stores the closing 0 (through 0x100ff030 when no byte is left). Returns what the
 * formatting engine returned.
 */
int __cdecl Crt_SPrintf(char *buf, const char *format, ...);
/*
 * fclose without locking: for a stream with flag 0x40 (a string stream) only clears the flags and returns -1.
 * Streams in use (0x83) are flushed (Crt_FlushStream), their buffer freed (Sim_10104aa0) and their handle closed
 * (0x101049c0); a close error gives -1, otherwise a temporary-name block (+0x1c) is freed. The flags are cleared.
 * Returns the flush result, -1 or -1 after a close error.
 */
int __cdecl Crt_FClose(void *stream);
/* _fsopen: takes a free stream (Sim_10104db0) and opens name on it (Sim_10104be0); 0 when no stream is free. */
uint8_t *__cdecl Crt_FSOpen(const char *name, const char *mode, uint32_t share);

#endif
