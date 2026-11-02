/* Checker stand-in for src/sim/crt/hg_format.c (which uses SDL3): the original DLL's formatting engine 0x10100fd0
 * and scanning engine 0x10103ba0, as the converted wrappers called them before the C runtime replacement. The specs
 * then compare the wrappers' own logic; SDL3's formatter is checked natively against the original's sprintf
 * (DECISIONS.md, "C runtime replacement: formatting"). */
#include <stdint.h>
#include <string.h>
#include "../../../src/sim/simmem.h"
#include "../../../src/sim/crt/hg_format.h"

int hg_vformat(char *buf, size_t size, const char *fmt, va_list ap)
{
    char big[0xf00];            /* on the stack, which the checker does not compare (below a page: no stack probe) */
    uint32_t file[8] = {0};     /* string stream: next byte, room, buffer, flags (0x42) */
    int n;
    size_t k;

    file[0] = file[2] = (uint32_t)(uintptr_t)big;
    file[1] = sizeof big - 1;
    file[3] = 0x42;
    n = SIM_FN(int (__cdecl *)(void *, const char *, va_list), 0x10100fd0)(file, fmt, ap);
    if (n < 0)
        n = (int)(sizeof big - 1);
    if (size > 0) {
        k = (size_t)n < size - 1 ? (size_t)n : size - 1;
        memcpy(buf, big, k);
        buf[k] = 0;
    }
    return n;
}

int hg_vscan(const char *text, const char *fmt, va_list ap)
{
    struct {
        uint32_t ptr, count, base, flags;
        uint32_t rest[4];
    } stream;

    stream.ptr = stream.base = (uint32_t)(uintptr_t)text;
    stream.count = (uint32_t)strlen(text);
    stream.flags = 0x49;
    return SIM_FN(int (__cdecl *)(void *, const char *, va_list), 0x10103ba0)(&stream, fmt, ap);
}
