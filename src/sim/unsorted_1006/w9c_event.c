#include "w9c.h"
#include "../unsorted_1002/w5a.h"
#include "../util/access.h"

/* switches the weapon record off: the player's cockpit message by the weapon kind +4, then the closed state */
static void W9c_WeaponOff(uint8_t *rec, int isCurrent)
{
    if (isCurrent) {
        switch (U32(rec, 4)) {
        case 0: case 1:
            Sim_100203cf(0x12, 3);
            break;
        case 2: case 3: case 4:
            Sim_100203cf(0x13, 3);
            break;
        case 5: case 7: case 0x2b:
            Sim_100203cf(0x17, 3);
            break;
        case 6:
            Sim_100203cf(0x1e, 3);
            break;
        case 8:
            Sim_100203cf(0x1a, 3);
            break;
        case 9: case 10: case 11:
            Sim_100203cf(0x1c, 3);
            break;
        case 12: case 13: case 14:
            Sim_100203cf(0x19, 3);
            break;
        case 15: case 16: case 17: case 18: case 19: case 20:
        case 21: case 22: case 23: case 24: case 25: case 26:
            Sim_100203cf(0x1d, 3);
            break;
        case 27: case 0x29: case 0x2a:
            Sim_100203cf(0x1f, 3);
            break;
        case 28: case 0x26: case 0x27:
            Sim_100203cf(0x18, 3);
            break;
        case 29: case 30: case 31: case 32:
            Sim_100203cf(0x1b, 3);
            break;
        case 33: case 34: case 35: case 0x28:
            Sim_100203cf(0x14, 3);
            break;
        case 36: case 0x2c:
            Sim_100203cf(0x16, 3);
            break;
        case 37: case 0x2d:
            Sim_100203cf(0x15, 3);
            break;
        default:
            break;
        }
    }
    U32(rec, 0) = 0;
    U32(rec, 8) = 0xffffffffu;
    U32(rec, 0x114) = 1;
    U32(rec, 0x20) = 0xfffffffeu;
}

// FUNCTION: HGSIM 0x1006bf33
void __cdecl Sim_1006bf33(uint32_t unused, int32_t code, void *s)
{
    int isCurrent = S32(U32(s, 0), 4) == GS32(0x1017b8c8);
    uint8_t *rec = BYTES(U32(s, 0x84));
    int i;

    (void)unused;
    for (i = 0; i < 8; i++, rec += 0x128) {
        if (S32(rec, 4) < 0)
            continue;
        if (S32(rec, 0x30) != code)
            continue;
        W9c_WeaponOff(rec, isCurrent);
        return;
    }
    /* Deliberate fix (DECISIONS.md round 22): no record carries the part code of a design built by 0x1005bb7a,
       so its weapon on the destroyed part is found through the part recorded at load time. */
    rec = Sim_FindWeaponMount(BYTES(s), code);
    if (rec != 0) {
        W9c_WeaponOff(rec, isCurrent);
        if (g_simWeaponOffObserver != 0)
            g_simWeaponOffObserver(BYTES(s), (int32_t)((rec - BYTES(U32(s, 0x84))) / 0x128), code);
    }
}
