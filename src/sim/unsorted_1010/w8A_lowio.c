#include <string.h>
#include "w8A.h"
#include "w7s.h"
#include "w6k.h"
#include "w2f.h"
#include "../util/access.h"
#include "../../platform/plat.h"

#define ADDR(p) ((uint32_t)(uintptr_t)(p))
#define ERRNO G32(0x1017e888)
#define DOSERRNO G32(0x1017e88c)
#define NHANDLES G32(0x10ac385c)
/* address of the block entry of fh in the handle table, and the slot offset inside the block */
#define BLOCKP(fh) (0x10ac3720u + 4u * (uint32_t)((int32_t)(fh) >> 5))
#define SLOTOFF(fh) (((fh) & 0x1fu) * 8u)
#define SLOT(fh) (G32(BLOCKP(fh)) + SLOTOFF(fh))

/* C runtime replacement, files (DECISIONS.md): the platform's file functions in place of Windows' (same shapes) */
#define GET_LAST_ERROR Plat_FileLastError
#define WRITE_FILE Plat_FileWrite
#define READ_FILE Plat_FileRead

// FUNCTION: HGSIM 0x10105fe0
int32_t __cdecl Crt_LSeek(uint32_t fh, int32_t offset, uint32_t origin)
{
    uint32_t h, pos, err;

    if (fh >= NHANDLES || (U8(SLOT(fh), 4) & 1) == 0) {
        ERRNO = 9;
        DOSERRNO = 0;
        return -1;
    }
    h = Sim_10109890(fh);
    if (h == 0xffffffffu) {
        ERRNO = 9;
        return -1;
    }
    pos = Plat_FileSeek(h, offset, 0, origin);
    err = pos == 0xffffffffu ? GET_LAST_ERROR() : 0;
    if (err != 0) {
        Sim_10105100(err);
        return -1;
    }
    U8(SLOT(fh), 4) &= 0xfd;
    return (int32_t)pos;
}

// FUNCTION: HGSIM 0x10105750
int32_t __cdecl Crt_Write(uint32_t fh, const uint8_t *buf, uint32_t cnt)
{
    uint8_t lbuf[0x404]; /* room for 0x400 bytes plus a '\r' '\n' pair, like the original's frame */
    uint32_t total = 0;  /* bytes WriteFile reported */
    uint32_t added = 0;  /* '\r' bytes put in by the text mode */
    uint32_t err = 0;
    uint32_t done;

    if (fh >= NHANDLES || (U8(SLOT(fh), 4) & 1) == 0) {
        ERRNO = 9;
        DOSERRNO = 0;
        return -1;
    }
    if (cnt == 0)
        return 0;
    if (U8(SLOT(fh), 4) & 0x20)
        Crt_LSeek(fh, 0, 2);
    if (U8(SLOT(fh), 4) & 0x80) {
        const uint8_t *p = buf;

        do {
            uint8_t *q = lbuf;
            uint32_t n;

            while ((uint32_t)(p - buf) < cnt) {
                uint8_t c = *p++;

                if (c == '\n') {
                    added++;
                    *q++ = '\r';
                }
                *q++ = c;
                if ((int32_t)(q - lbuf) >= 0x400)
                    break;
            }
            n = (uint32_t)(q - lbuf);
            if (!WRITE_FILE(U32(SLOT(fh), 0), lbuf, n, &done, 0)) {
                err = GET_LAST_ERROR();
                break;
            }
            total += done;
            if ((int32_t)done < (int32_t)n)
                break;
        } while ((uint32_t)(p - buf) < cnt);
    } else {
        if (WRITE_FILE(U32(SLOT(fh), 0), buf, cnt, &done, 0)) {
            err = 0;
            total = done;
        } else {
            err = GET_LAST_ERROR();
        }
    }
    if (total != 0)
        return (int32_t)(total - added);
    if (err != 0) {
        if (err == 5) {
            ERRNO = 9;
            DOSERRNO = 5;
            return -1;
        }
        Sim_10105100(err);
        return -1;
    }
    if ((U8(SLOT(fh), 4) & 0x40) && buf[0] == 0x1a)
        return 0;
    ERRNO = 0x1c;
    DOSERRNO = 0;
    return -1;
}

// FUNCTION: HGSIM 0x10105b50
int32_t __cdecl Crt_Read(uint32_t fh, uint8_t *buf, uint32_t cnt)
{
    uint32_t slot, done, err;
    uint8_t flags;
    int32_t nread = 0;
    uint8_t *p = buf;
    uint8_t *q, *r, *end;
    /* the original reads the look-ahead byte into its cnt argument slot, which still holds the cnt passed in */
    uint32_t peekslot = cnt;

    if (fh >= NHANDLES || (U8(SLOT(fh), 4) & 1) == 0) {
        ERRNO = 9;
        DOSERRNO = 0;
        return -1;
    }
    slot = SLOT(fh);
    flags = U8(slot, 4);
    if (cnt == 0 || (flags & 2))
        return 0;
    if ((flags & 0x48) && U8(slot, 5) != '\n') {
        *buf = U8(slot, 5);
        p = buf + 1;
        nread = 1;
        cnt--;
        U8(SLOT(fh), 5) = '\n';
    }
    if (!READ_FILE(U32(SLOT(fh), 0), p, cnt, &done, 0)) {
        err = GET_LAST_ERROR();
        if (err == 5) {
            DOSERRNO = 5;
            ERRNO = 9;
            return -1;
        }
        if (err == 0x6d)
            return 0;
        Sim_10105100(err);
        return -1;
    }
    nread += (int32_t)done;
    if ((U8(SLOT(fh), 4) & 0x80) == 0)
        return nread;
    if (done != 0 && buf[0] == '\n')
        U8(SLOT(fh), 4) |= 4;
    else
        U8(SLOT(fh), 4) &= 0xfb;
    end = buf + nread;
    q = buf;
    r = buf;
    while (r < end) {
        uint8_t c = *r;
        uint8_t peek;

        if (c == 0x1a) {
            if ((U8(SLOT(fh), 4) & 0x40) == 0)
                U8(SLOT(fh), 4) |= 2;
            break;
        }
        if (c != '\r') {
            *q++ = c;
            r++;
            continue;
        }
        if (r < end - 1) {
            if (r[1] == '\n') {
                r += 2;
                *q++ = '\n';
            } else {
                *q++ = '\r';
                r++;
            }
            continue;
        }
        /* '\r' as the last byte: look at the next byte of the file */
        r++;
        err = 0;
        if (!READ_FILE(U32(SLOT(fh), 0), &peekslot, 1, &done, 0))
            err = GET_LAST_ERROR();
        peek = (uint8_t)peekslot;
        if (err != 0 || done == 0) {
            *q++ = '\r';
        } else if (U8(SLOT(fh), 4) & 0x48) {
            if (peek == '\n') {
                *q++ = '\n';
            } else {
                *q++ = '\r';
                U8(SLOT(fh), 5) = peek;
            }
        } else if (q == buf && peek == '\n') {
            *q++ = '\n';
        } else {
            Crt_LSeek(fh, -1, 1);
            if (peek != '\n')
                *q++ = '\r';
        }
    }
    return (int32_t)(q - buf);
}

// FUNCTION: HGSIM 0x101049c0
int32_t __cdecl Crt_Close(uint32_t fh)
{
    uint32_t err;

    if (fh >= NHANDLES || (U8(SLOT(fh), 4) & 1) == 0) {
        ERRNO = 9;
        DOSERRNO = 0;
        return -1;
    }
    if (Sim_10109890(fh) == 0xffffffffu) {
        err = 0;
    } else if ((fh == 1 || fh == 2) && Sim_10109890(2) == Sim_10109890(1)) {
        err = 0;
    } else if (!Plat_FileClose(Sim_10109890(fh))) {
        err = GET_LAST_ERROR();
    } else {
        err = 0;
    }
    Sim_101097f0(fh);
    U8(SLOT(fh), 4) = 0;
    if (err != 0) {
        Sim_10105100(err);
        return -1;
    }
    return 0;
}

// FUNCTION: HGSIM 0x1010c330
int32_t __cdecl Crt_ChSize(uint32_t fh, int32_t size)
{
    uint8_t zeros[0x1000];
    int32_t ret = 0;
    int32_t pos, end, left, n, w, mode;

    if (fh >= NHANDLES || (U8(SLOT(fh), 4) & 1) == 0) {
        ERRNO = 9;
        return -1;
    }
    pos = Crt_LSeek(fh, 0, 1);
    if (pos == -1)
        return -1;
    end = Crt_LSeek(fh, 0, 2);
    if (end == -1)
        return -1;
    left = (int32_t)((uint32_t)size - (uint32_t)end);
    if (left > 0) {
        memset(zeros, 0, sizeof zeros);
        mode = Sim_1010cd90(fh, 0x8000);
        for (;;) {
            n = left >= 0x1000 ? 0x1000 : left;
            w = Crt_Write(fh, zeros, (uint32_t)n);
            if (w == -1) {
                if (DOSERRNO == 5)
                    ERRNO = 0xd;
                ret = -1;
                break;
            }
            left = (int32_t)((uint32_t)left - (uint32_t)w);
            if (left <= 0)
                break;
        }
        Sim_1010cd90(fh, mode);
    } else if (left < 0) {
        Crt_LSeek(fh, size, 0);
        ret = Plat_FileTruncate(Sim_10109890(fh)) ? 0 : -1;
        if (ret == -1) {
            ERRNO = 0xd;
            DOSERRNO = GET_LAST_ERROR();
        }
    }
    Crt_LSeek(fh, pos, 0);
    return ret;
}
