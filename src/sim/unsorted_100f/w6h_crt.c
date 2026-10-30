#include <stdarg.h>
#include <string.h>
#include "w6h.h"
#include "w2k.h"
#include "../util/access.h"

/* The address an original code location has where the DLL is loaded (what the relocated original stores). */
#define W6H_CODE(addr) ((uint32_t)(uintptr_t)SIM_PTR(uint8_t, addr))
/* The character class table pointer. */
#define W6H_CTYPE() G32(0x10159450)

// FUNCTION: HGSIM 0x100fc640
void __cdecl Sim_100fc640(void)
{
    G32(0x10159704) = W6H_CODE(0x10100af0);
    G32(0x10159700) = W6H_CODE(0x10100f30);
    G32(0x10159708) = W6H_CODE(0x10100b80);
    G32(0x1015970c) = W6H_CODE(0x10100a90);
    G32(0x10159710) = W6H_CODE(0x10100b60);
    G32(0x10159714) = W6H_CODE(0x10100f30);
}

// FUNCTION: HGSIM 0x100fc970
char *__cdecl Crt_StrStr(const char *hay, const char *needle)
{
    if (needle[0] == 0)
        return (char *)hay;
    if (needle[1] == 0)
        return Crt_StrChr(hay, (uint8_t)needle[0]);     /* the original jumps into strchr */
    for (; *hay != 0; hay++) {
        uint32_t i = 0;

        while (needle[i] != 0 && hay[i] == needle[i])
            i++;
        if (needle[i] == 0)
            return (char *)hay;
    }
    return 0;
}

// FUNCTION: HGSIM 0x100fd9a0
int __cdecl Crt_IsCType(int c, int mask)
{
    uint8_t bytes[4];
    uint16_t type[2];
    int n;

    if ((uint32_t)c + 1u <= 0x100u)
        return U16(W6H_CTYPE(), c * 2) & mask;
    if (U8(W6H_CTYPE(), (((uint32_t)c >> 8) & 0xff) * 2 + 1) & 0x80) {
        bytes[0] = (uint8_t)((uint32_t)c >> 8);
        bytes[1] = (uint8_t)c;
        bytes[2] = 0;
        n = 2;
    } else {
        bytes[0] = (uint8_t)c;
        bytes[1] = 0;
        n = 1;
    }
    if (SIM_CALL_10104fd0(1, (const char *)bytes, n, type, 0, 0) == 0)
        return 0;
    return type[0] & mask;
}

/* Class test of the two atol loops: the table byte, or Crt_IsCType when the dword 0x1015965c is above 1. */
static int W6h_Class(uint32_t c, int mask)
{
    if (GS32(0x1015965c) > 1)
        return Crt_IsCType((int)c, mask);
    return U8(W6H_CTYPE(), c * 2) & mask;
}

// FUNCTION: HGSIM 0x100fd110
int32_t __cdecl Crt_AToL(const char *s)
{
    const uint8_t *p = (const uint8_t *)s;
    uint32_t c;
    uint32_t sign;
    uint32_t total = 0;

    while (W6h_Class(*p, 8) != 0)
        p++;
    c = *p++;
    sign = c;
    if (c == '-' || c == '+')
        c = *p++;
    while (W6h_Class(c, 4) != 0) {
        total = c + total * 10u - 0x30u;
        c = *p++;
    }
    return (int32_t)(sign == '-' ? 0u - total : total);
}

// FUNCTION: HGSIM 0x100fd1b0
int32_t __cdecl Crt_AToI(const char *s)
{
    return Crt_AToL(s);
}

// FUNCTION: HGSIM 0x100fd290
int32_t __cdecl Sim_100fd290(const char *s, char **end, int base)
{
    return SIM_CALL_100fd2b0(s, end, base, 0);
}

// FUNCTION: HGSIM 0x100fd540
int32_t __cdecl Sim_100fd540(const char *s, char **end, int base)
{
    return SIM_CALL_100fd2b0(s, end, base, 1);
}

// FUNCTION: HGSIM 0x100fd710
int __cdecl Sim_100fd710(const char *s, const char *format, ...)
{
    /* The stream structure as the original builds it; its last four dwords stay unset there too. */
    struct {
        uint32_t ptr, count, base, flags;
        uint32_t rest[4];
    } stream;
    va_list ap;
    int result;

    stream.ptr = (uint32_t)(uintptr_t)s;
    stream.count = (uint32_t)strlen(s);
    stream.base = (uint32_t)(uintptr_t)s;
    stream.flags = 0x49;
    va_start(ap, format);
    result = SIM_CALL_10103ba0(&stream, format, (void *)ap);
    va_end(ap);
    return result;
}

// FUNCTION: HGSIM 0x100fd850
void *__cdecl Sim_100fd850(const char *name, const char *mode)
{
    return SIM_CALL_100fd820(name, mode, 0x40);
}
