#include <stdarg.h>
#include <string.h>
#include "w7q.h"
#include "w6i.h"
#include "../unsorted_1010/w6j.h"
#include "../util/access.h"

// FUNCTION: HGSIM 0x100fd7e0
int __cdecl Crt_FPrintf(void *stream, const char *format, ...)
{
    va_list args;
    int temp;
    int written;

    temp = Sim_10104ae0(stream);
    va_start(args, format);
    written = W7Q_OUTPUT(stream, format, (void *)args);
    va_end(args);
    Sim_10104b80(temp, stream);
    return written;
}

// FUNCTION: HGSIM 0x100fdc80
int __cdecl Crt_Printf(const char *format, ...)
{
    void *out = SIM_PTR(uint8_t, 0x10159b68);
    va_list args;
    int temp;
    int written;

    temp = Sim_10104ae0(out);
    va_start(args, format);
    written = W7Q_OUTPUT(out, format, (void *)args);
    va_end(args);
    Sim_10104b80(temp, out);
    return written;
}

// FUNCTION: HGSIM 0x100fe390
int __cdecl Crt_FPuts(const char *s, void *stream)
{
    uint32_t len = (uint32_t)strlen(s);
    int temp;
    uint32_t done;

    temp = Sim_10104ae0(stream);
    done = W7Q_WRITE_ITEMS(s, 1, len, stream);
    Sim_10104b80(temp, stream);
    return done == len ? 0 : -1;
}

// FUNCTION: HGSIM 0x100fe240
int __cdecl Crt_FFlush(void *stream)
{
    if (stream == 0)
        return Sim_100fe310(0);
    if (Crt_FlushStream(stream) != 0)
        return -1;
    if ((U32(stream, 0xc) & 0x4000u) != 0)
        return Sim_101056f0(U32(stream, 0x10)) != 0 ? -1 : 0;
    return 0;
}

// FUNCTION: HGSIM 0x100fe290
int __cdecl Crt_FlushStream(void *stream)
{
    uint32_t flags = U32(stream, 0xc);
    int result = 0;

    if ((flags & 3u) == 2 && (flags & 0x108u) != 0) {
        int count = (int)(U32(stream, 0) - U32(stream, 8));

        if (count > 0) {
            int done = W7Q_WRITE(U32(stream, 0x10), U32(stream, 8), count);

            flags = U32(stream, 0xc);
            if (done == count) {
                if ((flags & 0x80u) != 0) {
                    U32(stream, 4) = 0;
                    U32(stream, 0xc) = flags & 0xfffffffdu;
                    U32(stream, 0) = U32(stream, 8);
                    return 0;
                }
            } else {
                result = -1;
                U32(stream, 0xc) = flags | 0x20u;
            }
        }
    }
    U32(stream, 4) = 0;
    U32(stream, 0) = U32(stream, 8);
    return result;
}

/* The flag byte of a file handle in the handle table (blocks of 32 eight-byte entries at 0x10ac3720). */
static uint8_t W7q_HandleFlags(int32_t fd)
{
    return U8(G32(0x10ac3720u + (uint32_t)(fd >> 5) * 4u), (uint32_t)(fd & 0x1f) * 8u + 4u);
}

// FUNCTION: HGSIM 0x100fe970
int32_t __cdecl Crt_FTell(void *stream)
{
    int32_t fd = S32(stream, 0x10);
    int32_t pos;
    int32_t offset;
    uint32_t flags;
    uint32_t next;
    uint32_t buf;

    if (S32(stream, 4) < 0)
        U32(stream, 4) = 0;
    pos = W7Q_SEEK(fd, 0, 1);
    if (pos < 0)
        return -1;
    flags = U32(stream, 0xc);
    if ((flags & 0x108u) == 0)
        return pos - S32(stream, 4);
    next = U32(stream, 0);
    buf = U32(stream, 8);
    offset = (int32_t)(next - buf);
    if ((flags & 3u) != 0) {
        if ((W7q_HandleFlags(fd) & 0x80) != 0) {
            uint32_t q;

            for (q = buf; q < next; q++)
                if (U8(q, 0) == '\n')
                    offset++;
        }
    } else if ((flags & 0x80u) == 0) {
        G32(0x1017e888) = 0x16;
        return -1;
    }
    if (pos == 0)
        return offset;
    if ((U8(stream, 0xc) & 1) != 0) {
        uint32_t left = U32(stream, 4);
        uint32_t read;

        if (left == 0)
            return pos;
        read = next + left - buf;
        if ((W7q_HandleFlags(fd) & 0x80) != 0) {
            if (W7Q_SEEK(fd, 0, 2) == pos) {
                uint32_t p = U32(stream, 8);
                uint32_t stop = p + read;

                for (; p < stop; p++)
                    if (U8(p, 0) == '\n')
                        read++;
                if ((U32(stream, 0xc) & 0x2000u) != 0)
                    read++;
            } else {
                W7Q_SEEK(fd, pos, 0);
                flags = U32(stream, 0xc);
                if (read <= 0x200 && (flags & 8u) != 0 && (flags & 0x400u) == 0)
                    read = 0x200;
                else
                    read = U32(stream, 0x18);
                if ((W7q_HandleFlags(fd) & 4) != 0)
                    read++;
            }
        }
        pos -= (int32_t)read;
    }
    return pos + offset;
}
