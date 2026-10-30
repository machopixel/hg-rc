#include "w9c.h"
#include "../util/access.h"

// FUNCTION: HGSIM 0x10057d2d
void __cdecl Key_SetState(uint32_t wparam, uint32_t lparam, int down)
{
    int32_t scan = ((int32_t)lparam >> 16) & 0xff;
    uint32_t *bitmap = SIM_PTR(u32a, 0x1015e650);
    uint32_t *mods = SIM_PTR(u32a, 0x1015e65c);
    uint32_t bit;

    (void)wparam;
    if (scan != 0 && ((lparam >> 16) & 0x100) != 0 && scan < 0x59)
        scan = SIM_PTR(s32a, 0x10143c00)[scan];
    if (scan == 0)
        return;
    scan -= 1;
    bit = 1u << (scan & 0x1f);
    if (down != 0)
        bitmap[scan / 32] |= bit;
    else
        bitmap[scan / 32] &= ~bit;
    switch (scan) {
    case 0x1c:
    case 0x6f:
        if (down != 0)
            *mods |= 0x800000u;
        else
            *mods &= 0xff7fffffu;
        break;
    case 0x29:
    case 0x35:
        if (down != 0)
            *mods |= 0x400000u;
        else
            *mods &= 0xffbfffffu;
        break;
    case 0x37:
    case 0x70:
        if (down != 0)
            *mods |= 0x1000000u;
        else
            *mods &= 0xfeffffffu;
        break;
    default:
        break;
    }
}
