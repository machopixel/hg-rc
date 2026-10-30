#include "w7q.h"
#include "../unsorted_1003/w6e.h"
#include "../unsorted_1002/w5a.h"
#include "../util/access.h"

// FUNCTION: HGSIM 0x100d191d
void __cdecl Sim_100d191d(void *node)
{
    if (node == 0)
        return;
    U32(node, 0) = 0;
    Sim_100d191d((void *)(uintptr_t)U32(node, 0xc));
    Sim_100d191d((void *)(uintptr_t)U32(node, 8));
}

// FUNCTION: HGSIM 0x100d1b91
void __cdecl Sim_100d1b91(int option, uint32_t value)
{
    switch (option) {
    case 0x2b:
        G32(0x10144fa0) = value;
        break;
    case 0x54:
        G32(0x101452d0) = value;
        break;
    case 0x58:
        G32(0x1015f1d8) = value;
        break;
    case 0xc4:
        Sim_1003cbb3(0, (int)value);
        break;
    case 0xc5:
        if (value != 0) {
            G32(0x1014cb1c) = 1;
            Sim_100203cf(1, 1);
        } else {
            G32(0x1014cb1c) = 0;
        }
        break;
    default:
        break;
    }
}
