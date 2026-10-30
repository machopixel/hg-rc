#include <string.h>
#include "w8A.h"
#include "w7q.h"
#include "../unsorted_1010/w8A.h"
#include "../unsorted_1010/w7s.h"
#include "../unsorted_1010/w6k.h"
#include "../unsorted_1010/w2k.h"
#include "../util/access.h"

#define ADDR(p) ((uint32_t)(uintptr_t)(p))
#define S_PTR(s) U32(s, 0)
#define S_CNT(s) U32(s, 4)
#define S_BASE(s) U32(s, 8)
#define S_FLAG(s) U32(s, 0xc)
#define S_FILE(s) U32(s, 0x10)
#define S_BUFSIZ(s) U32(s, 0x18)

typedef int32_t(__stdcall *W8aDeleteFileAFn)(const char *path);
typedef uint32_t(__stdcall *W8aGetLastErrorFn)(void);

// FUNCTION: HGSIM 0x100fe830
uint32_t __cdecl Crt_FRead(void *buf, uint32_t size, uint32_t count, uint8_t *stream)
{
    uint8_t *p = buf;
    uint32_t total = size * count;
    uint32_t left = total;
    uint32_t bufsize;

    if (total == 0)
        return 0;
    bufsize = (S_FLAG(stream) & 0x10c) ? S_BUFSIZ(stream) : 0x1000;
    do {
        if ((S_FLAG(stream) & 0x10c) && S_CNT(stream) != 0) {
            uint32_t n = S_CNT(stream);

            if (left < n)
                n = left;
            memcpy(p, (const void *)(uintptr_t)S_PTR(stream), n);
            left -= n;
            S_CNT(stream) -= n;
            S_PTR(stream) += n;
            p += n;
        } else if (left >= bufsize) {
            uint32_t n = bufsize != 0 ? left - left % bufsize : left;
            int32_t r = Crt_Read(S_FILE(stream), p, n);

            if (r == 0) {
                S_FLAG(stream) |= 0x10;
                return (total - left) / size;
            }
            if (r == -1) {
                S_FLAG(stream) |= 0x20;
                return (total - left) / size;
            }
            left -= (uint32_t)r;
            p += r;
        } else {
            int32_t c = Sim_10105a60(stream);

            if (c == -1)
                return (total - left) / size;
            *p++ = (uint8_t)c;
            left--;
            bufsize = S_BUFSIZ(stream);
        }
    } while (left != 0);
    return count;
}

// FUNCTION: HGSIM 0x100febc0
uint32_t __cdecl Crt_FWrite(const void *buf, uint32_t size, uint32_t count, uint8_t *stream)
{
    const uint8_t *p = buf;
    uint32_t total = size * count;
    uint32_t left = total;
    uint32_t bufsize;

    if (total == 0)
        return 0;
    bufsize = (S_FLAG(stream) & 0x10c) ? S_BUFSIZ(stream) : 0x1000;
    do {
        uint32_t buffered = S_FLAG(stream) & 0x108;

        if (buffered && S_CNT(stream) != 0) {
            uint32_t n = S_CNT(stream);

            if (left < n)
                n = left;
            memcpy((void *)(uintptr_t)S_PTR(stream), p, n);
            left -= n;
            S_CNT(stream) -= n;
            S_PTR(stream) += n;
            p += n;
        } else if (left >= bufsize) {
            uint32_t n;
            int32_t w;

            if (buffered && Crt_FlushStream(stream) != 0)
                return (total - left) / size;
            n = bufsize != 0 ? left - left % bufsize : left;
            w = Crt_Write(S_FILE(stream), p, n);
            if (w == -1) {
                S_FLAG(stream) |= 0x20;
                return (total - left) / size;
            }
            left -= (uint32_t)w;
            p += w;
            if ((uint32_t)w < n) {
                S_FLAG(stream) |= 0x20;
                return (total - left) / size;
            }
        } else {
            if (Crt_FlushBuf((int8_t)*p, stream) == -1)
                return (total - left) / size;
            p++;
            left--;
            bufsize = S_BUFSIZ(stream);
            if ((int32_t)bufsize <= 0)
                bufsize = 1;
        }
    } while (left != 0);
    return count;
}

// FUNCTION: HGSIM 0x100feb20
int32_t __cdecl Crt_FSeek(uint8_t *stream, int32_t offset, uint32_t origin)
{
    uint32_t f = S_FLAG(stream);

    if ((f & 0x83) == 0 || (origin != 0 && origin != 1 && origin != 2)) {
        G32(0x1017e888) = 0x16;
        return -1;
    }
    S_FLAG(stream) = f & ~0x10u;
    if (origin == 1) {
        offset = (int32_t)((uint32_t)offset + (uint32_t)Crt_FTell(stream));
        origin = 0;
    }
    Crt_FlushStream(stream);
    f = S_FLAG(stream);
    if (f & 0x80)
        S_FLAG(stream) = f & ~3u;
    else if ((f & 1) && (f & 8) && (f & 0x400) == 0)
        S_BUFSIZ(stream) = 0x200;
    return Crt_LSeek(S_FILE(stream), offset, origin) == -1 ? -1 : 0;
}

// FUNCTION: HGSIM 0x100ff030
int32_t __cdecl Crt_FlushBuf(int32_t ch, void *stream)
{
    uint8_t *s = stream;
    uint32_t f = S_FLAG(s);
    uint32_t fh = S_FILE(s);
    int32_t n, written;

    if ((f & 0x82) == 0 || (f & 0x40)) {
        S_FLAG(s) = f | 0x20;
        return -1;
    }
    if (f & 1) {
        S_CNT(s) = 0;
        if ((f & 0x10) == 0) {
            S_FLAG(s) = f | 0x20;
            return -1;
        }
        S_PTR(s) = S_BASE(s);
        S_FLAG(s) = f & ~1u;
    }
    f = S_FLAG(s);
    S_CNT(s) = 0;
    f = (f & ~0x10u) | 2u;
    S_FLAG(s) = f;
    if ((f & 0x10c) == 0) {
        if ((ADDR(s) != 0x10159b68 && ADDR(s) != 0x10159b88) || Sim_10107080(fh) == 0)
            Sim_10107020(s);
    }
    if (S_FLAG(s) & 0x108) {
        uint32_t base = S_BASE(s);

        n = (int32_t)(S_PTR(s) - base);
        S_PTR(s) = base + 1;
        S_CNT(s) = S_BUFSIZ(s) - 1;
        if (n > 0) {
            written = Crt_Write(fh, (const uint8_t *)(uintptr_t)base, (uint32_t)n);
            U8(S_BASE(s), 0) = (uint8_t)ch;
        } else {
            uint32_t slot = fh == 0xffffffffu ? 0x10159dc8u
                                              : G32(0x10ac3720u + 4u * (uint32_t)((int32_t)fh >> 5)) + (fh & 0x1fu) * 8u;

            written = 0;
            if (U8(slot, 4) & 0x20)
                Crt_LSeek(fh, 0, 2);
            U8(S_BASE(s), 0) = (uint8_t)ch;
        }
    } else {
        uint8_t c = (uint8_t)ch;

        n = 1;
        written = Crt_Write(fh, &c, 1);
    }
    if (written != n) {
        S_FLAG(s) |= 0x20;
        return -1;
    }
    return ch & 0xff;
}

// FUNCTION: HGSIM 0x100fe3e0
int32_t __cdecl Crt_VSNPrintf(char *buf, uint32_t count, const char *fmt, void *args)
{
    uint32_t file[8] = {0}; /* string stream: next byte, room, buffer, flags (0x42) */
    int32_t n;

    file[2] = ADDR(buf);
    file[0] = ADDR(buf);
    file[1] = count;
    file[3] = 0x42;
    n = W8A_FN_10100fd0(file, fmt, args);
    file[1]--;
    if ((int32_t)file[1] >= 0)
        *(char *)(uintptr_t)file[0] = 0;
    else
        Crt_FlushBuf(0, file);
    return n;
}

// FUNCTION: HGSIM 0x100fdb30
int32_t __cdecl Crt_Remove(const char *path)
{
    uint32_t err = SIM_API(W8aDeleteFileAFn, 0x10ac56f0)(path) ? 0 : SIM_API(W8aGetLastErrorFn, 0x10ac5600)();

    if (err != 0) {
        Sim_10105100(err);
        return -1;
    }
    return 0;
}
