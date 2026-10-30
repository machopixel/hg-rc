#include "w9e.h"

// FUNCTION: HGSIM 0x10075753
void __cdecl Sim_10075753(int code, uint32_t sub, uint32_t *outA, uint32_t *outB)
{
    if (code <= 0xd) {
        if (code == 0xd) {
            switch (sub) {
            case 0:
                *outA = 3;
                break;
            case 2:
                *outA = 4;
                break;
            case 5:
            case 6:
                *outA = 5;
                break;
            default:
                break;
            }
            return;
        }
        if (code == 9)
            *outB = 1;
        return;
    }
    if (code <= 0xc6) {
        if (code == 0xc6)
            *outB = 0xffffffffu;
        else if (code == 0x1b)
            *outA = 5;
        return;
    }
    if (code == 0xc7)
        *outB = 1;
    else if (code == 0x209)
        *outB = 0xffffffffu;
}
