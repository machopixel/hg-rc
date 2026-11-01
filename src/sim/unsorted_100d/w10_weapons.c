#include "w10.h"
#include "../simmem.h"
#include "../util/access.h"
#include "../util/w3o.h"
#include "w2f.h"
#include "w2g.h"
#include "w7q.h"
#include "../unsorted_1000/w5a.h"
#include "../unsorted_1000/w8t.h"
#include "../unsorted_1002/w5a.h"
#include "../unsorted_1002/w5y.h"
#include "../unsorted_100c/w8z.h"

/* a weapon kind's routines in its 0xd6-byte record: +0 runs every frame, +4 fires and returns the rounds used */
typedef void(__cdecl *W10dWeaponTickFn)(void *obj, void *weapon, void *slot);
typedef int(__cdecl *W10dWeaponFireFn)(void *obj, void *weapon, void *slot);

#define W10D_OBJ(u) BYTES(U32(u, 0))
#define W10D_CTL(u) BYTES(U32(W10D_OBJ(u), 0x54))
#define W10D_SLOT(u, i) (BYTES(U32(u, 0x84)) + (uint32_t)(i) * 0x128u)
#define W10D_WEAPON(id) (SIM_PTR(uint8_t, 0x10145508) + (uint32_t)(id) * 0xd6u)
#define W10D_NOW() GS32(0x1015dc78)

/* one of the trigger bytes +0x28..+0x2b of the controls held */
static int W10d_TriggerHeld(uint8_t *u)
{
    return U8(W10D_CTL(u), 0x28) != 0 || U8(W10D_CTL(u), 0x29) != 0 || U8(W10D_CTL(u), 0x2a) != 0 ||
           U8(W10D_CTL(u), 0x2b) != 0;
}

/* the selected slot fired from trigger byte k (0x29..0x2b): flag 1 of +0x560 once, Sim_100d60b6 and the fire sound */
static void W10d_Group(uint8_t *u, int group)
{
    if ((U16(u, 0x560) & 1) == 0) {
        U16(u, 0x560) |= 1;
        if (Sim_100d60b6(u, group) != 0)
            Sim_100d650f(u);
    }
}

#ifndef W10F_KEEP_SEL
#define W10F_KEEP_SEL 1
#endif

// FUNCTION: HGSIM 0x100d4590
// FUNCTION: HGSIM12 0x100ba550
void __cdecl Sim_100d4590(uint8_t *u)
{
    uint32_t tbl = U32(W10D_OBJ(u), 0x48);
    uint8_t *slot;
    uint8_t *weapon;
    int i;

    if (U32(u, 0x4f0) != 2)
        return;
    U32(u, 0x508) = U32(u, 0x4fc);
    if (U8(W10D_CTL(u), 0x2d) != 0) {
        if (G32(0x1015243c) != 0) {
            G32(0x1015243c) = 0;
            Sim_100203cf(0x21, 1);
        } else {
            G32(0x1015243c) = 1;
            Sim_100203cf(0x21, 0);
        }
    }

    if ((U16(u, 0x560) & 0x4000) != 0) {
        Sim_100d67af(u);
    } else if (S32(u, 0x4fc) != -1) {
        slot = W10D_SLOT(u, U32(u, 0x4fc));
        if (G32(0x1015243c) == 0 && U32(W10D_OBJ(u), 4) == G32(0x1017b8c8) && U8(W10D_CTL(u), 0x2c) == 0) {
            U8(W10D_CTL(u), 0x28) = U8(W10D_CTL(u), 0x26);
            U8(W10D_CTL(u), 0x26) = 0;
        }
        if (U8(W10D_CTL(u), 0x28) != 0) {
            if ((U16(u, 0x560) & 1) == 0) {
                U16(u, 0x560) |= 1;
                Sim_100d650f(u);
            }
        } else if (U8(W10D_CTL(u), 0x2e) != 0) {
            if ((U16(u, 0x560) & 1) == 0) {
                U16(u, 0x560) |= 1;
                Sim_100d666d(u);
            }
            U8(W10D_CTL(u), 0x2e) = 0;
        } else if (U8(W10D_CTL(u), 0x29) != 0) {
            W10d_Group(u, 0);
        } else if (U8(W10D_CTL(u), 0x2a) != 0) {
            W10d_Group(u, 1);
        } else if (U8(W10D_CTL(u), 0x2b) != 0) {
            W10d_Group(u, 2);
        } else if (U8(W10D_CTL(u), 0x26) != 0) {
            if ((U16(u, 0x560) & 1) == 0) {
                U16(u, 0x560) |= 1;
                weapon = W10D_WEAPON(U32(slot, 4));
                if (U32(slot, 8) == 1) {
                    if (U32(weapon, 0x10) == 0x3d && S32(slot, 0x20) != -2)
                        U16(u, 0x562) |= 0x800;
                    if ((U16(u, 0x562) & 0x800) == 0)
                        U32(slot, 8) = 2;
                    U32(slot, 0x2c) = U32(weapon, 0x2c);
                    U32(slot, 0x10) = 0;
                    Sim_100c398d(slot, u);
                    if ((U32(weapon, 0x86) & 1) != 0 && (U16(u, 0x560) & 0x80) != 0) {
                        U32(slot, 0x18) = U32(W10D_OBJ(u), 0xf0) & 0xff;
                        U32(slot, 0x1c) = U32(W10D_OBJ(u), 0xf0) & 0xf00;
                    } else {
                        U32(slot, 0x1c) = 0;
                        U32(slot, 0x18) = 0;
                    }
                } else if (U32(W10D_OBJ(u), 4) == G32(0x1017b8c8)) {
                    if (S32(slot, 8) == -1)
                        Sim_1000ebfb(SIM_PTR(const char, 0x10152440), (uint32_t)-1000, 0, 5, 0x32);
                    else
                        Sim_1000ebfb(SIM_PTR(const char, 0x1015244c), (uint32_t)-3000, 0, 5, 0x32);
                }
            }
        } else if ((U16(u, 0x560) & 1) != 0) {
            U16(u, 0x560) &= 0xfffe;
        }
    }

    for (i = 0; i < 8; i++) {
        int32_t t;

        slot = W10D_SLOT(u, i);
        if (S32(slot, 4) < 0)
            continue;
        weapon = W10D_WEAPON(U32(slot, 4));
        if (U32(weapon, 0x10) == 0x3d && (U16(u, 0x562) & 0x1000) != 0 && S32(slot, 0x20) != -2) {
            U32(slot, 8) = 2;
            U16(u, 0x562) &= 0xefff;
        }
        if (U32(weapon, 0) != 0)
            ((W10dWeaponTickFn)(uintptr_t)U32(weapon, 0))(W10D_OBJ(u), weapon, slot);
        switch (U32(slot, 8)) {
        case 0:
            if (W10D_NOW() - S32(slot, 0x10) >= S32(weapon, 0x54)) {
                U32(slot, 8) = 1;
                U32(slot, 0x10) = 0;
            }
            break;
        case 3:
            if (W10D_NOW() - S32(slot, 0x10) >= S32(weapon, 0x54)) {
                if (W10D_SLOT(u, U32(u, 0x4fc)) == slot || W10d_TriggerHeld(u))
                    U32(slot, 8) = 2;
                else
                    U32(slot, 8) = 1;
                U32(slot, 0x10) = 0;
            }
            break;
        case 2:
            if (S32(slot, 0x2c) <= 0) {
                U32(slot, 0x10) = (uint32_t)W10D_NOW();
                U32(slot, 8) = 0;
                U32(slot, 0x2c) = 0;
                break;
            }
            if (U32(slot, 0x2c) == U32(weapon, 0x2c))
                G32(0x1017e67c) = 1;
            t = GS32(0x1015dc80) + S32(slot, 0x10);
            while (t >= S32(weapon, 0x58) && S32(slot, 0x2c) > 0) {
                int32_t used = 0;

                U32(W10D_OBJ(u), 0x50) = U32(u, 0x498 + U32(slot, 0x11c) * 4);
                if (U32(weapon, 4) != 0)
                    used = ((W10dWeaponFireFn)(uintptr_t)U32(weapon, 4))(W10D_OBJ(u), weapon, slot);
                if (used == 0) {
                    U16(u, 0x560) &= 0xfffe;
                    U32(slot, 0x2c) = 0;
                    continue;
                }
                /* 1.2 (0x100ba550) makes the slot that fires the current weapon, so the rest of a burst switches a
                   player who has just changed weapons back to the old one. Bug fix (Diego, 2026-11-01: "keep my new
                   selection"): not for the local player's gear; computer gears keep 1.2's behaviour. W10F_KEEP_SEL 0
                   builds 1.2's line (the checker's comparison). */
#if W10F_KEEP_SEL
#ifdef W10F_MUTATE /* the checker's negative check only: the fix for every gear */
                if (0)
#else
                if (U32(W10D_OBJ(u), 4) != G32(0x1017b8c8))
#endif
#endif
                    U32(u, 0x4fc) = (uint32_t)i;
                if (U32(W10D_OBJ(u), 0) == 9)
                    Sim_10024684(0x1e, U32(u, 0));
                else if (U8(BYTES(U32(tbl, 0x90)), 0) == 0x53 && U32(weapon, 0x18) != 0x1a)
                    Sim_10024684(0x4c, U32(u, 0));
                else
                    Sim_10024684(U32(weapon, 0x18), U32(u, 0));
                U32(slot, 0x2c) -= (uint32_t)used;
                t -= S32(weapon, 0x58);
                U32(slot, 0x10) = (uint32_t)t;
                if ((U32(weapon, 8) & 1) != 0 || (U32(weapon, 8) & 2) != 0) {
                    G32(0x1017e67c) = 0;
                    U32(u, 0xbf5 + U32(slot, 0x104) * 4) = 1;
                    if (U8(weapon, 0xb2) != 0) {
                        uint32_t loud = (U32(weapon, 8) & 2) != 0 ? 0x432 : 0x32;

                        if (U32(W10D_OBJ(u), 4) == G32(0x1017b8c8) && G32(0x1015ef0c) != 0) {
                            Sim_1000ebfb((const char *)(weapon + 0xb2), 0, 0, 5, loud);
                        } else {
                            uint8_t *cam = BYTES(G32(0x1014cae0));
                            float dx = (float)((double)FL(cam, 0) - (double)FL(W10D_OBJ(u), 0x58));
                            float dy = (float)((double)FL(cam, 4) - (double)FL(W10D_OBJ(u), 0x5c));
                            float dz = (float)((double)FL(cam, 8) - (double)FL(W10D_OBJ(u), 0x60));

                            Sim_1000eb12(dx, dy, dz, (const char *)(weapon + 0xb2), (int)G32(0x1015ef0c), loud,
                                         U32(u, 0));
                        }
                    }
                }
                if (U32(slot, 0x2c) == 0 && U32(weapon, 0x20) != 0 &&
                    (U8(W10D_CTL(u), 0x26) != 0 || W10d_TriggerHeld(u))) {
                    U32(slot, 0x10) = (uint32_t)W10D_NOW();
                    U32(slot, 8) = 3;
                    U32(slot, 0x2c) = U32(weapon, 0x2c);
                    break;
                }
            }
            if (U32(slot, 8) != 3)
                U32(slot, 0x10) = (uint32_t)t;
            break;
        }
    }
}
