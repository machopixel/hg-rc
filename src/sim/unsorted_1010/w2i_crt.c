#include "w2i.h"
#include "../util/access.h"

// FUNCTION: HGSIM 0x10101a70
uint32_t __cdecl Sim_10101a70(uint32_t *cursor)
{
    uint32_t next = *cursor + 4u;

    *cursor = next;
    return (next & 0xFFFF0000u) | U16(next - 4u, 0);
}

// FUNCTION: HGSIM 0x10103800
uint32_t __cdecl Sim_10103800(uint32_t key)
{
    uint32_t p = 0x101598B0u;
    uint32_t i = 0;

    do {
        if (G32(p) == key)
            return G32(0x101598B4u + i * 8u);
        p += 8;
        i++;
    } while ((int32_t)p < 0x10159988);
    return 0;
}

// FUNCTION: HGSIM 0x10106d80
int __cdecl Sim_10106d80(void *hdr, void *region, uint8_t *block, uint32_t newSize)
{
    uint32_t reg = (uint32_t)(uintptr_t)region;
    uint32_t addr = (uint32_t)(uintptr_t)block;
    uint32_t size = U8(addr, 0);
    uint32_t entry = (uint32_t)(uintptr_t)hdr + (uint32_t)((int32_t)(reg - U32(hdr, 0x10)) >> 12) * 8u + 0x18u;
    uint32_t end, p, cursor;

    if (size > newSize) {
        uint32_t counter = U32(entry, 0);

        U8(addr, 0) = (uint8_t)newSize;
        U32(entry, 4) = 0xf1;
        U32(entry, 0) = counter + (size - newSize);
        return 1;
    }
    if (size >= newSize)
        return 0;
    end = addr + newSize;
    if (end > reg + 0xf8u)
        return 0;
    p = addr + size;
    if (p < end) {
        do {
            if (U8(p, 0) != 0)
                break;
            p++;
        } while (p < end);
    }
    if (p != end)
        return 0;
    U8(addr, 0) = (uint8_t)newSize;
    cursor = U32(reg, 0);
    if (addr <= cursor && end > cursor) {
        if (end < reg + 0xf8u) {
            uint32_t run = 0;

            U32(reg, 0) = end;
            if (U8(end, 0) == 0) {
                uint8_t next;

                do {
                    next = U8(end + run, 1);
                    run++;
                } while (next == 0);
            }
            U32(reg, 4) = run;
        } else {
            U32(reg, 4) = 0;
            U32(reg, 0) = reg + 8u;
        }
    }
    U32(entry, 0) = U32(entry, 0) + (size - newSize);
    return 1;
}

// FUNCTION: HGSIM 0x101071a0
char *__cdecl Text_PutTwoDigits(char *dst, int n)
{
    dst[0] = (char)(n / 10 + '0');
    dst[1] = (char)(n % 10 + '0');
    return dst + 2;
}

// FUNCTION: HGSIM 0x101070b0
char *__cdecl Time_FormatTm(const int32_t *tm)
{
    uint32_t weekday = 0x1012C440u + (uint32_t)S32(tm, 0x18) * 3u;
    uint32_t month = 0x1012C458u + (uint32_t)S32(tm, 0x10) * 3u;
    char *p;

    GBYTES(0x1017E97B)[0] = 0x20;
    GBYTES(0x1017E97F)[0] = 0x20;
    U16(GBYTES(0x1017E97C), 0) = U16(GBYTES(month), 0);
    U16(GBYTES(0x1017E978), 0) = U16(GBYTES(weekday), 0);
    U8(GBYTES(0x1017E97E), 0) = U8(GBYTES(month), 2);
    U8(GBYTES(0x1017E97A), 0) = U8(GBYTES(weekday), 2);
    p = Text_PutTwoDigits((char *)GBYTES(0x1017E980), S32(tm, 0xc));
    *p++ = ' ';
    p = Text_PutTwoDigits(p, S32(tm, 8));
    *p++ = ':';
    p = Text_PutTwoDigits(p, S32(tm, 4));
    *p++ = ':';
    p = Text_PutTwoDigits(p, S32(tm, 0));
    *p++ = ' ';
    p = Text_PutTwoDigits(p, S32(tm, 0x14) / 100 + 19);
    p = Text_PutTwoDigits(p, S32(tm, 0x14) % 100);
    p[0] = '\n';
    p[1] = 0;
    return (char *)GBYTES(0x1017E978);
}

// FUNCTION: HGSIM 0x10107f70
uint32_t __cdecl Sim_10107f70(uint32_t codePage)
{
    switch (codePage) {
    case 932:
        return 0x411;
    case 936:
        return 0x804;
    case 949:
        return 0x412;
    case 950:
        return 0x404;
    default:
        return 0;
    }
}

// FUNCTION: HGSIM 0x101083b0
void __cdecl Sim_101083b0(uint32_t value)
{
    G32(0x1017E900) = value;
}
