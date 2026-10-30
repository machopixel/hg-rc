#include "w6e.h"
#include "../unsorted_1005/w2k.h"
#include "../util/access.h"

// FUNCTION: HGSIM 0x10075124
void __cdecl Sim_10075124(uint8_t *obj)
{
    uint32_t block = U32(obj, 0xc);

    if (block != 0) {
        if (U32(block, 0x14) != 0) {
            W6E_CALL_100bb2b0(U32(block, 0x10), G32(0x1014cec0));
            U32(block, 0x14) = 0;
        }
        if (U32(block, 0x20) != 0) {
            W6E_CALL_100bb2b0(U32(block, 0x1c), G32(0x1014cec0));
            U32(block, 0x20) = 0;
        }
        if (U32(block, 0x48) != 0) {
            W6E_CALL_100bb2b0(U32(block, 0x44), G32(0x1014cec4));
            U32(block, 0x48) = 0;
        }
        if ((U32(block, 4) & 1) != 0)
            Sim_1005571d();
    }
    U32(obj, 0xc) = 0;
    G32(0x1015dbcc) = G32(0x1015dbcc) - 1;
}

/* The C library isspace test of the original: _isctype for multi-byte locales, else the class table [0x10159450]. */
static int IsSpace(char c)
{
    if (GS32(0x1015965c) > 1)
        return W6E_CALL_100fd9a0((int)(int8_t)c, 8);
    return U16(G32(0x10159450), (uint32_t)((int32_t)(int8_t)c * 2)) & 8;
}

// FUNCTION: HGSIM 0x10079a80
void __cdecl Str_CollapseSpaces(char *s)
{
    char *src = s;
    char *dst = s;

    while (*src != 0) {
        *dst++ = *src++;
        if (IsSpace(*src)) {
            while (*src != 0 && IsSpace(*src))
                src++;
            *dst++ = ' ';
        }
    }
    *dst = 0;
}
