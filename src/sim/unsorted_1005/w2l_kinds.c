#include "w2l.h"

// FUNCTION: HGSIM 0x10057a4a
int __cdecl Sim_10057a4a(void)
{
    return 0;
}

// FUNCTION: HGSIM 0x1005b89b
int __cdecl Sim_1005b89b(int kind, int other)
{
    if ((kind >= 2 && kind <= 8) || kind == 0xd || kind == 0xe || (kind >= 0x21 && kind <= 0x25) || kind == 0x28)
        return 0x100;
    if (kind >= 0x1d && kind <= 0x20)
        return 0x200;
    return other;
}
