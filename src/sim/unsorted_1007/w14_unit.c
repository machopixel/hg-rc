#include "w14.h"
#include "w2e.h"
#include "w2g.h"
#include "../simmem.h"
#include "../util/access.h"
#include "../util/util.h"
#include "../util/w3o.h"
#include "../view/view.h"
#include "../unsorted_1000/w2e.h"
#include "../unsorted_1000/w5a.h"
#include "../unsorted_1000/w5x.h"
#include "../unsorted_1001/w8t.h"
#include "../unsorted_1002/w5a.h"
#include "../unsorted_1004/fns.h"
#include "../unsorted_1004/w10.h"
#include "../unsorted_1004/w5z.h"
#include "../unsorted_1006/w10.h"
#include "../unsorted_1006/w5b.h"
#include "../unsorted_1006/w6e.h"
#include "../unsorted_1006/w8w.h"
#include "../unsorted_1008/w3o.h"
#include "../unsorted_1008/w8w.h"
#include "../unsorted_100c/w10.h"
#include "../unsorted_100c/w5x.h"
#include "../unsorted_100c/w5y.h"
#include "../unsorted_100c/w5z.h"
#include "../unsorted_100c/w8z.h"
#include "../unsorted_100c/w9g.h"
#include "../unsorted_100d/fns.h"
#include "../unsorted_100d/w10.h"
#include "../unsorted_1004/w2k.h"
#include "../render/w2n.h"
#include "../unsorted_100d/w2i.h"
#include "../libm/hg_math.h"

/* fabs and sprintf of the DLL (left original). */
#define W14U_FABS hg_fabs
#define W14U_SPRINTF SIM_FN(int(__cdecl *)(char *, const char *, ...), 0x100fc780)

/* The unit and its control block, read again at every use as the original does. */
#define W14U_UNIT BYTES(U32(rec, 0))
#define W14U_BLK BYTES(U32(W14U_UNIT, 0x54))
#define W14U_C(addr) ((double)GFL(addr))
#define W14U_D(addr) (*SIM_PTR(double, addr))
/* The x87 "equal" flag of fcomp: equal or unordered. */
#define W14U_EQ(a, b) (!((a) < (b)) && !((a) > (b)))
/* Word +0x562 bits 0x60e (legs or engine broken) or +0xc3d: the gear cannot drive. */
#define W14U_STUCK() ((U16(rec, 0x562) & 0x60e) != 0 || U32(rec, 0xc3d) != 0)
#define W14U_OWNED() (S32(W14U_UNIT, 0x14) == 3 && U32(W14U_UNIT, 0xc) == G32(0x1017b8c8))

/* The control requests of the block, each cleared after it is handled (two clear the wrong byte, as the original). */
static void W14U_Requests(uint8_t *rec, int player)
{
    if (U8(W14U_BLK, 0x4c) != 0) {
        U32(rec, 0x4d8) = G32(0x1015dc78) + 0x16a;
        U32(rec, 0x4f0) = 7;
        U8(W14U_BLK, 0x4c) = 0;
    }
    if (U8(W14U_BLK, 0x47) != 0)
        Sim_100d6392(0);
    else if (U8(W14U_BLK, 0x48) != 0)
        Sim_100d6392(1);
    else if (U8(W14U_BLK, 0x49) != 0)
        Sim_100d6392(2);
    else if (U8(W14U_BLK, 0x4a) != 0)
        Sim_100d6392(3);
    else if (U8(W14U_BLK, 0x4b) != 0)
        Sim_100d6392(4);
    if (U8(W14U_BLK, 0x31) != 0) {
        Sim_100cbdb8(U32(rec, 0), 1, 0);
        U8(W14U_BLK, 0x31) = 0;
    }
    if (U8(W14U_BLK, 0x32) != 0) {
        Sim_100cbdb8(U32(rec, 0), 0xffffffffu, 0);
        U8(W14U_BLK, 0x32) = 0;
    }
    if (U8(W14U_BLK, 0x33) != 0) {
        Sim_100cbdb8(U32(rec, 0), 0, 0);
        U8(W14U_BLK, 0x33) = 0;
    }
    if (U8(W14U_BLK, 0x34) != 0) {
        Sim_100ca670(W14U_UNIT, 1, 8);
        U8(W14U_BLK, 0x34) = 0;
    }
    if (U8(W14U_BLK, 0x35) != 0) {
        Sim_100ca670(W14U_UNIT, -1, 8);
        U8(W14U_BLK, 0x35) = 0;
    }
    if (U8(W14U_BLK, 0x36) != 0) {
        Sim_100ca670(W14U_UNIT, 0, 8);
        U8(W14U_BLK, 0x35) = 0;
    }
    if (U8(W14U_BLK, 0x3c) != 0) {
        Sim_100cbe94();
        U8(W14U_BLK, 0x3c) = 0;
    } else if (player && (U32(rec, 0xbcb) & 1) != 0) {
        if (G32(0x1017cc5c) == 0) {
            G32(0x10179cf4) = 0;
        } else if (G32(0x10179cf4) == 0) {
            Sim_100cbe94();
            G32(0x10179cf4) = 1;
        }
    }
    if (U8(W14U_BLK, 0x38) != 0) {
        Sim_100cbe3c(1);
        U8(W14U_BLK, 0x38) = 0;
    }
    if (U8(W14U_BLK, 0x39) != 0) {
        Sim_100cbe3c(0xffffffffu);
        U8(W14U_BLK, 0x39) = 0;
    }
    if (U8(W14U_BLK, 0x3a) != 0) {
        Sim_100cbe68(1);
        U8(W14U_BLK, 0x3a) = 0;
    }
    if (U8(W14U_BLK, 0x3b) != 0) {
        Sim_100cbe68(0xffffffffu);
        U8(W14U_BLK, 0x3b) = 0;
    }
    if (U8(W14U_BLK, 0x3d) != 0)
        U8(W14U_BLK, 0x3d) = 0;
    if (U8(W14U_BLK, 0x3f) != 0) {
        Sim_100ca670(W14U_UNIT, 1, 0x10008);
        U8(W14U_BLK, 0x3f) = 0;
    }
    if (U8(W14U_BLK, 0x40) != 0) {
        Sim_100cbdea(1);
        U8(W14U_BLK, 0x40) = 0;
    }
    if (U8(W14U_BLK, 0x41) != 0) {
        Sim_100cbdea(0xffffffffu);
        U8(W14U_BLK, 0x41) = 0;
    }
    if (U8(W14U_BLK, 0x42) != 0) {
        Sim_100cbdea(0);
        U8(W14U_BLK, 0x41) = 0;
    }
    if (U8(W14U_BLK, 0x43) != 0) {
        Sim_100cbe13(1);
        U8(W14U_BLK, 0x43) = 0;
    }
    if (U8(W14U_BLK, 0x44) != 0) {
        Sim_100cbe13(0xffffffffu);
        U8(W14U_BLK, 0x44) = 0;
    }
    if (U8(W14U_BLK, 0x45) != 0) {
        Sim_100cbe13(0);
        U8(W14U_BLK, 0x44) = 0;
    }
    if (U8(W14U_BLK, 0x37) != 0) {
        Sim_100cbaf8();
        U8(W14U_BLK, 0x37) = 0;
    }
}

/* A boost or gear request of the player: the engine rate +0x78 / +0x7c and the block's +8 / +0x25. */
static void W14U_Gear(uint8_t *rec, int player, uint8_t dir, uint32_t clear)
{
    U8(W14U_BLK, 0x59) = dir;
    FL(rec, 0x78) = (float)(W14U_C(0x1012b6bc) * (double)FL(rec, 0x500) + W14U_C(0x1012b6bc));
    U32(rec, 0x7c) = U32(rec, 0x78);
    U8(W14U_BLK, clear) = 0;
    U32(W14U_BLK, 8) = 0x3c800000u;
    U8(W14U_BLK, 0x25) = 1;
    if (player)
        G32(0x10179ce8) = 1;
}

/* The throttle and engine rate of a running unit the player controls (or any in single player). */
static void W14U_Drive(uint8_t *rec, int player)
{
    W14U_Requests(rec, player);
    if ((U32(W14U_UNIT, 0xf0) & 0x1000) == 0)
        Sim_100cb4fb(W14U_UNIT);
    Sim_100d6833(rec);
    if ((W14U_FABS((double)FL(W14U_BLK, 0x10)) >= W14U_C(0x1012b6c4) || G32(0x1015f1e0) != 0) &&
        U32(rec, 0x50c) != 0)
        U8(W14U_BLK, 0x46) = 1;
    if (U8(W14U_BLK, 0x46) != 0) {
        if (U32(rec, 0xc35) == 0) {
            U32(rec, 0x50c) = 6;
            Sim_1000ebfb(SIM_PTR(const char, 0x10145470), 0, 0, 5, 0x32);
        } else if (U32(rec, 0x50c) == 0) {
            U32(rec, 0x50c) = 1;
            if (U32(W14U_UNIT, 4) == G32(0x1017b8c8))
                Sim_100203cf(2, 1);
        } else {
            U32(rec, 0x50c) = 0;
        }
    }
    U8(W14U_BLK, 0x46) = 0;
    if (S32(W14U_UNIT, 0x14) == 0 && U32(rec, 0x50c) == 0)
        U32(W14U_BLK, 0xc) = U32(W14U_BLK, 0x10);
    if (W14U_STUCK())
        U32(W14U_BLK, 0xc) = 0;
    if (S32(W14U_UNIT, 0x14) == 0)
        Sim_10069c90(rec);
    if (!W14U_STUCK()) {
        FL(rec, 0x78) = (float)((double)FL(W14U_BLK, 8) * (double)FL(rec, 0x500) + W14U_C(0x1012b6bc));
        if (player) {
            if (U8(W14U_BLK, 0x5a) != 0)
                U8(W14U_BLK, 0x59) ^= 1;
            if (G32(0x10179ce8) != 0) {
                U32(rec, 0x78) = 0x3c800000u;
                U32(rec, 0x7c) = 0x3c800000u;
                U32(W14U_BLK, 8) = 0;
                U8(W14U_BLK, 0x25) = 1;
                if (player)
                    G32(0x10179ce8) = 0;
            }
            if (U8(W14U_BLK, 0x50) != 0) {
                U8(W14U_BLK, 0x59) = 0;
                FL(rec, 0x78) = (float)((double)FL(rec, 0x500) * W14U_D(0x1012b760) + W14U_D(0x1012b768));
                U32(rec, 0x7c) = U32(rec, 0x78);
                U8(W14U_BLK, 0x50) = 0;
                U32(W14U_BLK, 8) = 0x3c000000u;
                U8(W14U_BLK, 0x25) = 1;
                if (player)
                    G32(0x10179ce8) = 1;
            } else if (U8(W14U_BLK, 0x51) != 0) {
                W14U_Gear(rec, player, 1, 0x51);
            } else if (U8(W14U_BLK, 0x52) != 0) {
                W14U_Gear(rec, player, 0, 0x50);
            } else if (U8(W14U_BLK, 0x53) != 0) {
                W14U_Gear(rec, player, 1, 0x51);
            }
        }
    } else {
        U32(W14U_BLK, 8) = 0;
        U32(rec, 0x78) = 0;
    }
    if (S32(W14U_UNIT, 0x14) != 2 && !W14U_OWNED() && W14U_EQ((double)FL(rec, 0x78), W14U_C(0x1012b6bc)) &&
        U32(rec, 0xc3d) == 0 &&
        (W14U_FABS((double)FL(W14U_BLK, 0xc)) >= W14U_C(0x1012b6c4) || G32(0x1015f1e0) != 0) &&
        (U16(rec, 0x562) & 0x60e) == 0)
        U32(rec, 0x78) = 0x3c900000u;
}

/* The block's throttle +0xc scaled by the unit kind: / 1024 times 180 (kinds 8, 2), else times 90 and the factor. */
static float W14U_Throttle(uint8_t *rec, int32_t kind, uint32_t side2, uint32_t fast, uint32_t slow)
{
    double t = (double)FL(W14U_BLK, 0xc) / W14U_C(0x1012b774);

    if (kind == 8 || kind == 2)
        return (float)(t * W14U_C(0x1012b778));
    if (kind == 1 && S32(W14U_UNIT, 0x14) == 2)
        return (float)(t * W14U_C(0x1012b728) * W14U_C(side2));
    if ((U32(rec, 0xbd3) & 0x200) != 0)
        return (float)(t * W14U_C(0x1012b728) * W14U_C(fast));
    if (slow == 0)
        return (float)(t * W14U_C(0x1012b728));
    return (float)(t * W14U_C(0x1012b728) * W14U_C(slow));
}

/* Wraps an angle of more than 180 or less than -180 degrees by 360. */
static float W14U_Wrap(float a)
{
    if (a > GFL(0x1012b778))
        return (float)-(W14U_C(0x1012b790) - (double)a);
    if (!(a >= GFL(0x1012b794)))
        return (float)(W14U_C(0x1012b790) + (double)a);
    return a;
}

/* The auto-turn of the player towards the heading kept in [0x10179cdc]. */
static void W14U_AutoTurn(uint8_t *rec)
{
    float a = FL(W14U_UNIT, 0x68), aw, b, d;

    a = W14U_Wrap(a);
    aw = a;
    if (GFL(0x10179cdc) > GFL(0x1012b778) || !(GFL(0x10179cdc) >= GFL(0x1012b794)))
        GFL(0x10179cdc) = W14U_Wrap(GFL(0x10179cdc));
    b = GFL(0x10179cdc);
    if (aw > GFL(0x1012b728) && !(b >= GFL(0x1012b72c)))
        a = (float)-(W14U_C(0x1012b790) - (double)a);
    else if (b > GFL(0x1012b728) && !(aw >= GFL(0x1012b72c)))
        a = (float)(W14U_C(0x1012b790) + (double)a);
    d = (float)((double)a - (double)GFL(0x10179cdc));
    if (FL(rec, 0xc) > GFL(0x1012b798) && !(FL(rec, 0xc) >= GFL(0x1012b79c))) {
        U32(rec, 0x38) = 0;
        U32(rec, 0x3c) = 0;
        U32(rec, 8) = 0;
        U32(rec, 0xc) = 0;
        *GBYTES(0x1015fd54) = 1;
        U32(W14U_BLK, 4) = 0;
        G32(0x1015f1e0) = 0;
        return;
    }
    if (FL(rec, 0xc) > GFL(0x1012b6c4))
        U32(rec, 0x38) = (U16(rec, 0x562) & 1) != 0 ? 0x41c80000u : 0x425c0000u;
    else
        U32(rec, 0x38) = (U16(rec, 0x562) & 1) != 0 ? 0xc1c80000u : 0xc25c0000u;
    if (!W14U_EQ((double)d, W14U_C(0x1012b6dc))) {
        FL(rec, 8) = (float)((double)FL(rec, 0xc) - (double)d);
        U32(rec, 0xc) = U32(rec, 8);
    }
}

// FUNCTION: HGSIM 0x10070eb5
void __cdecl Sim_10070eb5(uint8_t *rec)
{
    int player;
    int32_t kind;
    float pitch = 0.0f, throttle, speed, lean, f;      /* pitch: 1.2 sets its [ebp-0x18] to 0 at the start */
    double t;
    char text[40];

    if ((U16(rec, 0x560) & 0x200) != 0)
        return;
    player = U32(W14U_UNIT, 4) == G32(0x1017b8c8);
    kind = S32(W14U_UNIT, 0);
    if (S32(rec, 0x4f0) == 2) {
        Sim_100d4590(rec);
        if (U8(W14U_BLK, 0x2c) != 0 && player && U8(W14U_BLK, 0x28) != 0)
            Sim_100d63d4();
    }
    if (GS32(0x1014b930) >= 0 && GS32(0x1014b934) >= 0 && U32(W14U_UNIT, 4) == G32(0x1017b8c8)) {
        Sim_1006b52f(-1, rec, GS32(0x1014b934), GS32(0x1014b930), 0);
        G32(0x1014b930) = 0xffffffffu;
    }
    if (S32(W14U_UNIT, 0x14) == 2 || W14U_OWNED())
        Sim_10011681(W14U_UNIT);
    else if (G32(0x10a3ade8) != 0)
        Sim_1001349a();
    if ((G32(0x1015ec58) == 0 || W14U_OWNED() || player) && S32(rec, 0x4f0) == 2)
        W14U_Drive(rec, player);
    Sim_1004f257(W14U_UNIT);

    /* the jump jets */
    if (S32(rec, 0x510) == -2 && U8(W14U_BLK, 0x4d) != 0) {
        U32(rec, 0x510) = 0x2d4;
        U32(rec, 0x514) = 10;
        FL(rec, 0x540) = (float)(W14U_C(0x1012b6c8) * W14U_C(0x1013abc0));
    }
    if (S32(rec, 0x510) >= 0 && U32(rec, 0x514) != 0 && (U8(W14U_BLK, 0x1e) != 0 || U32(rec, 0x518) != 0) &&
        S32(rec, 0x4f0) == 2) {
        U32(rec, 0x518) = 1;
        if (player && S32(rec, 0x510) == 0x2d4) {
            Sim_1000ec60(SIM_PTR(const char, 0x1014547c), 0, 0, 5, 0x150);
            Sim_1000ebfb(SIM_PTR(const char, 0x10145488), 0, 0, 5, 0x32);
        }
        if (G32(0x10179cb8) == 0 || !player)
            U32(rec, 0x510) -= G32(0x1015dc80);
        if (S32(rec, 0x510) <= 0) {
            Sim_1000ecb0(SIM_PTR(const char, 0x10145494));
            U32(rec, 0x510) = 0x2d4;
            U32(rec, 0x514) -= 1;
            U32(rec, 0x518) = 0;
        }
    }

    /* the forward speed +0x38 */
    if ((U16(rec, 0x562) & 1) != 0 || U32(rec, 0x518) != 0) {
        if (S32(rec, 0x510) > 0 && U32(rec, 0x518) != 0)
            Sim_10061dfc((uint32_t *)rec);
        U32(W14U_UNIT, 0x88) &= 0xfffffffbu;
        U32(W14U_UNIT, 0x88) &= 0xfffffffeu;
        t = (double)FL(rec, 0x2c) * W14U_C(0x1012b6fc) * W14U_C(0x1012b6a8);
        pitch = (float)t;
        if (W14U_FABS(t) >= W14U_C(0x1012b6c4) || (player && FL(rec, 0x500) > GFL(0x1012b6c4))) {
            if (pitch > GFL(0x1012b770))
                pitch = 80.0f;
            throttle = W14U_Throttle(rec, kind, 0x1012b6cc, 0x1012b77c, 0x1012b6e0);
            f = Sim_10084766((float)((double)pitch + W14U_C(0x1012b728)));
            FL(rec, 0x38) = (float)((double)throttle * (double)f);
        } else {
            U32(rec, 0x38) = 0;
        }
    } else if ((U32(W14U_UNIT, 0x88) & 4) == 0) {
        t = (double)FL(rec, 0x2c) * W14U_C(0x1012b6fc);
        pitch = (float)t;
        if (!W14U_EQ(t, W14U_C(0x1012b6dc)) || (player && FL(rec, 0x500) > GFL(0x1012b6c4))) {
            if (pitch > GFL(0x1012b770))
                pitch = 80.0f;
            throttle = W14U_Throttle(rec, kind, 0x1012b748, 0x1012b780, 0);
            f = Sim_10084766((float)((double)pitch + W14U_C(0x1012b728)));
            speed = (float)((double)throttle * (double)f);
            FL(rec, 0x38) = speed;
            if (Sim_10079feb(kind) != 0) {
                t = (double)speed * (double)pitch * W14U_D(0x1012b788);
                lean = (float)t;
                if (t > W14U_C(0x1012b728))
                    lean = 90.0f;
                else if (!(lean >= GFL(0x1012b72c)))
                    lean = -90.0f;
                FL(rec, 0x48) = -lean;
            }
        } else {
            U32(rec, 0x38) = 0;
            if (Sim_10079feb(kind) != 0)
                U32(rec, 0x48) = 0;
        }
    } else if (U32(rec, 0x50c) == 0) {
        U32(rec, 0x38) = 0;
        if (Sim_10079feb(kind) != 0)
            U32(rec, 0x48) = 0;
    }
    /* 1.2 (0x10060491): the player's force-feedback rumble (kind 0x10) from the pitch, stronger with the jets */
    if (player && G32(0x1015ef3c) != 0) {
#ifdef W14UF_MUTATE /* the checker's negative check only: the same strength either way */
        if (0)
#else
        if ((U16(rec, 0x562) & 1) != 0)
#endif
            Sim_100382fb(0x10, 0, 0, (uint32_t)Render_Ftol((double)pitch * 1.25 + 30.0));
        else
            Sim_100382fb(0x10, 0, 0, (uint32_t)Render_Ftol((double)pitch * 1.25 + 15.0));
    }

    if (player && G32(0x1015f1e0) != 0)
        W14U_AutoTurn(rec);
    if (player)
        G32(0x10179cdc) = U32(W14U_UNIT, 0x68);

    /* the speed +0x28 */
    t = (double)FL(rec, 0x7c) - W14U_C(0x1012b6bc);
    f = (float)t;
    if (!(t >= W14U_C(0x1012b6ac)))
        f = 0.0f;
    FL(rec, 0x28) = (float)((double)f * (double)FL(rec, 0x4d4));
    if (kind == 8)
        FL(rec, 0x28) = (float)((double)FL(rec, 0x28) * W14U_C(0x1012b7a0));
    if (kind == 5) {
        float range = (float)Sim_10084a94(FL(rec, 0x548), 0.0f, FL(rec, 0x550));
        float full, share;

        t = (double)FL(rec, 0x4d4) * W14U_C(0x1012b6bc) * W14U_C(0x1012b6c0);
        full = (float)t;
        if (!W14U_EQ(t, W14U_C(0x1012b6dc))) {
            t = (double)range / (double)full;
            share = (float)t;
            if (t > W14U_C(0x1012b79c))
                share = 1.0f;
        } else {
            share = 0.0f;
        }
        f = (float)((double)share * W14U_C(0x1012b6b8));
        if (U8(W14U_BLK, 0x59) != 0)
            FL(rec, 0x58) = -f;
        else
            FL(rec, 0x58) = f;
    }
    if (W14U_FABS((double)FL(rec, 0x28)) >= W14U_C(0x1012b6c4) && U8(W14U_BLK, 0x59) != 0) {
        FL(rec, 0x28) = (float)((double)FL(rec, 0x28) / W14U_C(0x1012b6a8));
        FL(rec, 0x28) = (float)((double)FL(rec, 0x28) * W14U_C(0x1012b798));
    }
    if ((U32(W14U_UNIT, 0x88) & 2) != 0) {
        if (G32(0x10179cc4) != 0 && player && (int32_t)(G32(0x1015dc78) - G32(0x10179d00)) > 0xb5) {
            G32(0x10179d00) = G32(0x1015dc78);
            if (Sim_1009c929(0x3c) == 0xc && U8(G32(0x1017b8c4), 1) == 0 && S32(rec, 0x4f0) == 2) {
                /* the message is "MASC malfunction." (17 characters, DECISIONS.md round 14) */
                W14U_SPRINTF(text, (const char *)(uintptr_t)G32(0x101c4648));
                Sim_1007df6c(text, (const char *)(uintptr_t)G32(0x109b98c0), 0x16a, 0x32);
                FL(rec, 0x4e8) = (float)((double)FL(rec, 0x4e8) / W14U_C(0x1012b7a0) + (double)FL(rec, 0x4e8));
                G32(0x10179cc4) = 0;
            }
        }
        U32(rec, 0x30) = 0x42b50000u;
        if (G32(0x10179ce4) != 0 && U32(W14U_UNIT, 4) == G32(0x1017b8c8))
            FL(rec, 0x28) = (float)((double)FL(rec, 0x28) / W14U_C(0x1012b6a8) + (double)FL(rec, 0x28));
    }

    /* the steering +8, limited to +0x534 */
    if (G32(0x1015f1e0) == 0 && (player || G32(0x1015ec58) == 0 || W14U_OWNED()))
        U32(rec, 8) = U32(W14U_BLK, 4);
    if (FL(rec, 8) > FL(rec, 0x534)) {
        U32(rec, 8) = U32(rec, 0x534);
        U32(W14U_BLK, 4) = U32(rec, 0x534);
        U8(W14U_BLK, 0x1b) = 1;
    } else if (-FL(rec, 0x534) > FL(rec, 8)) {
        FL(rec, 8) = -FL(rec, 0x534);
        FL(W14U_BLK, 4) = -FL(rec, 0x534);
        U8(W14U_BLK, 0x1b) = 1;
    }

    /* the turn +0x18, with the player's pending offset [0x10179cec] */
    if (player && GFL(0x10179cec) > GFL(0x1012b6c4)) {
        if (!((double)FL(W14U_BLK, 0) - (double)GFL(0x10179cec) >= W14U_C(0x1012b7a4)))
            GFL(0x10179cec) = (float)((double)FL(W14U_BLK, 0) - (double)GFL(0x10179cec) + W14U_C(0x1012b7a8) +
                                      (double)GFL(0x10179cec));
        FL(rec, 0x18) = (float)((double)FL(W14U_BLK, 0) - (double)GFL(0x10179cec));
        G32(0x10179cec) = 0;
    } else {
        if (player)
            G32(0x10179cec) = 0;
        U32(rec, 0x18) = U32(W14U_BLK, 0);
    }
    if ((kind == 4 || kind == 10) && FL(rec, 0x18) >= GFL(0x1012b6dc) && FL(rec, 0x18) > GFL(0x1012b6dc))
        FL(rec, 0x18) = (float)((double)FL(rec, 0x18) * W14U_C(0x1012b7ac));
    FL(rec, 0x4e0) = (float)(((double)FL(rec, 0x7c) - W14U_C(0x1012b6bc)) * (double)FL(rec, 0x4ec) /
                                 W14U_C(0x1012b7b0) +
                             (double)FL(rec, 0x4e0));

    if (S32(rec, 0x4f0) != 2) {
        static const uint8_t cleared[] = {0x1f, 0x20, 0x21, 0x22, 0x26, 0x27, 0x2f, 0x30, 0x59,
                                          0x31, 0x46, 0x50, 0x53, 0x52, 0x51, 0x54, 0x56};
        unsigned i;

        Sim_1004f158(W14U_UNIT);
        U32(rec, 0x78) = 0x3c800000u;
        U32(rec, 0x28) = 0;
        U32(rec, 8) = 0;
        U32(rec, 0x38) = 0;
        U32(rec, 0x18) = 0;
        U32(W14U_BLK, 8) = 0;
        U8(W14U_BLK, 0x25) = 1;
        for (i = 0; i < sizeof(cleared); i++)
            U8(W14U_BLK, cleared[i]) = 0;
        if (U32(W14U_UNIT, 4) == G32(0x1017b8c8))
            Sim_100d4500(rec);
    }

    switch (U32(rec, 0x4f0)) {
    case 0:
        U32(rec, 0x4d8) = G32(0x1015dc78) + (uint32_t)Sim_1009c929(0x16a) + 0x43e;
        U32(rec, 0x4f0) = 1;
        U16(W14U_UNIT, 0x1c) &= 0xdfff;
        /* 1.2: the start-up sound for the player's own unit only (1.0: for any not answered 1 by Sim_10052be7),
           with the force-feedback kind 0xe */
#ifdef W14US_MUTATE /* the checker's negative check only: 1.0's test */
        if (Sim_10052be7(S32(W14U_UNIT, 4)) != 1) {
#else
        if (player) {
#endif
            Sim_1000ebfb(SIM_PTR(const char, 0x101454a0), 0xfffff830u, 0, 5, 0x32);
            if (G32(0x1015ef3c) != 0)
                Sim_100382fb(0xe, 0, 0, 0);
        }
        break;
    case 1:
        if (GS32(0x1015dc78) > S32(rec, 0x4d8)) {
            U32(rec, 0x4f0) = 2;
            U16(W14U_UNIT, 0x1c) |= 0x2000;
            if (G32(0x10179ce0) == 0) {
                if (player)
                    Sim_1000ebfb(SIM_PTR(const char, 0x101454ac), 0, 0, 5, 0x32);
                G32(0x10179ce0) = 1;
            }
        }
        break;
    case 2:
        if ((U16(W14U_UNIT, 0x1c) & 0x10) != 0) {
            U16(W14U_UNIT, 0x1c) &= 0xffef;
            U16(W14U_UNIT, 0x1c) |= 0x2000;
        }
        break;
    case 3:
        U16(W14U_UNIT, 0x1c) |= 0x10;
        break;
    case 4:
        U16(W14U_UNIT, 0x1c) &= 0xdfff;
        if (S32(W14U_UNIT, 0) == 9) {
            U16(rec, 0x560) |= 0x200;
        } else {
            if (U32(rec, 0x4d8) == 0) {
                Sim_10061c32((uint32_t *)rec);
                U32(rec, 0x4d8) = G32(0x1015dc78) + 0x712;
                Sim_1007cbb4();
            }
            if (GS32(0x1015dc78) < S32(rec, 0x4d8))
                Sim_10061c32((uint32_t *)rec);
            else
                U16(rec, 0x560) |= 0x200;
        }
        break;
    case 7:
        if (GS32(0x1015dc78) > S32(rec, 0x4d8))
            Sim_1006c9a0(rec, 0);
        break;
    }
    if (!player)
        U16(W14U_UNIT, 0x1c) &= 0xfffe;
}
