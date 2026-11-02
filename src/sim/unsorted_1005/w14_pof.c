#include "w14.h"
#include "w2k.h"
#include "w2l.h"
#include "w3s.h"
#include "w5x.h"
#include "../simmem.h"
#include "../util/access.h"
#include "../util/w3o.h"
#include "../render/w2n.h"
#include "../unsorted_1004/fns.h"
#include "../unsorted_1004/w2b.h"
#include "../unsorted_1004/w2c.h"
#include "../unsorted_1004/w2h.h"
#include "../unsorted_1004/w2j.h"
#include "../unsorted_1004/w2k.h"
#include "../unsorted_1004/w2l.h"
#include "../unsorted_1004/w3o.h"
#include "../unsorted_1004/w3p.h"
#include "../unsorted_1004/w3q.h"
#include "../unsorted_1004/w3t.h"
#include "../unsorted_1004/w5x.h"
#include "../unsorted_1004/w9h.h"
#include "../unsorted_100a/w5y.h"
#include "../unsorted_100f/w2e.h"
#include "../unsorted_100f/w6h.h"
#include "../unsorted_1006/w9c.h"
#include "../libm/hg_math.h"

/* The debug print of the DLL, by its address with the original's arguments (the damage log hooks it there). */
#define W14P_PRINT SIM_FN(void(__cdecl *)(const char *, const char *, const uint8_t *), 0x10083495)
#define W14P_ROW(sub, off) G32(0x10111600u + (uint32_t)(sub) * 0xaau + (off))
#define W14P_COLUMN(i) GS32(0x10124eacu + (uint32_t)(i) * 0x98u)

// FUNCTION: HGSIM 0x1005bb7a
// FUNCTION: HGSIM12 0x1004e6a0
int __cdecl Sim_1005bb7a(uint8_t *obj, uint8_t *design)
{
    const uint8_t *base = design + 0x8c;
    int32_t parts = S32(base, 0x24), i, j;
    int32_t links = 0, mounts = 0, col = 0, col2 = 0, armour = 0, value, top = 0, sensors = 0, cockpit = 0;
    int32_t weight = 0, x18 = 0, total = 0, speed = 0, jets = 0;
    uint32_t c35 = 0, bb7 = 0, bb3 = 0, baf = 0, bab = 0, bbf = 0, bc3 = 0, bc7 = 0, flags = 0;
    /* Set by the chassis part (kind 2) only; every shipped design has exactly one with a sub-kind 0..15
     * (DECISIONS.md round 14), so these start values only matter for a design without one. */
    int32_t class_ = 0, chassis = 0, chassis_row = 0, pof = 0;
    float jump = 0.0f, scale, lim;
    uint8_t *slot, *link, *part, *e;
    char name[12];

    U8(obj, 0x594) = 0;
    U16(obj, 0x562) = 0;
    U16(obj, 0x560) = 0;
    U32(obj, 0x5ff) = 0;
    U32(obj, 0xbcb) = U32(base, 0x30);
    U32(obj, 0xbcf) = U32(base, 0x34);
    U32(obj, 0xbd3) = U32(base, 0x38);
    U32(obj, 0x60b) = 0xb5;
    U32(obj, 0x613) = U32(base, 0x2c);
    U32(obj, 0xc15) = 0;
#ifndef W14PW_MUTATE /* the checker's negative check only: 1.0, the slot left as it was */
    S32(obj, 0x5fb) = -1;       /* 1.2 (0x1004e6a0): no selected weapon slot yet (the mount 0x10038ad5 takes the first) */
#endif
    for (i = 0; i < 0x28; i++)
        U8(obj, 0x498 + i) = 0;
    slot = BYTES(U32(obj, 0x84));
    link = BYTES(U32(obj, 0x8c));
    part = BYTES(U32(design, 0xc8));
    for (i = 0; i < parts; i++, part += 0x6c) {
        int32_t kind = S32(part, 0), sub = S32(part, 4), size = S32(part, 8);
        uint32_t c = U32(part, 0xc);

        if (kind != 4 && kind != 6)
            total += size;
        value = 0;
        switch (kind) {
        case 1:
            value = Sim_1004ce50(sub);
            cockpit += Sim_1004ce64(sub, c);
            sensors += Sim_1004cfb5(sub, c);
            weight += Sim_1004d106(sub, c);
            bb7 = (uint32_t)Sim_1004ce3d(c);
            c35 = (uint32_t)Sim_1004cdd1(c);
            bb3 = (uint32_t)Sim_1004ce04(c);
            baf = (uint32_t)Sim_1004cd9e(c);
            bab = (uint32_t)Sim_1004cd8d(c);
            bc3 = (uint32_t)Sim_1004cdf3(c);
            bc7 = (uint32_t)Sim_1004cde2(c);
            break;
        case 2:
            chassis = size;
            chassis_row = (int32_t)W14P_ROW(sub, 0xae);
            armour += Sim_1004c085(sub, c);
            sensors += Sim_1004c0e5(sub, (int)c, chassis);
            speed += Sim_1004c161(sub, c);
            top += Sim_1004c1c1(sub, c);
            weight += Sim_1004c221(sub, (int)c);
            x18 += Sim_1004c297(sub, c);
            col = S32(part, 0x64);
            col2 = S32(part, 0x68);
            value = Sim_1004bfbd(sub, (int)c);
            jets = Sim_1004bfa9((int)c);
            if (Sim_1004bf87((int)c) == 9)
                bbf = 1;
            switch (sub) {
            case 0:
            case 1:
            case 8:
            case 9:
                class_ = 1;
                jump = 0.25f;
                break;
            case 2:
            case 10:
                class_ = 2;
                jump = 0.283f;
                break;
            case 3:
            case 4:
            case 11:
            case 12:
                class_ = 3;
                jump = 0.316f;
                break;
            case 5:
            case 6:
            case 13:
            case 14:
                class_ = 4;
                jump = 0.35f;
                break;
            case 7:
            case 15:
                class_ = 5;
                jump = 0.0f;
                break;
            }
            break;
        case 3:
            value = Sim_1004c5c1(sub, c);
            armour += Sim_1004c6f5(sub, c);
            weight += Sim_1004c785(sub, c);
            x18 += Sim_1004c815(sub, c);
            flags |= (uint32_t)Sim_1004c5b7();
            break;
        case 4:
            value = Sim_1004c5c1(sub, c);
            break;
        case 5:
            value = Sim_1004c965(sub, (int)c);
            speed += Sim_1004ca99(sub, c);
            top += Sim_1004cb29(sub, c);
            weight += Sim_1004cbb9(sub, c);
            x18 += Sim_1004cc49(sub, c);
            flags |= (uint32_t)Sim_1004c95b();
            break;
        case 6:
            value = Sim_1004c965(sub, (int)c);
            break;
        case 7: {
            int r = Sim_1004c30d((int)c);

            if (r == 0x2d)
                U32(obj, 0x5cb) = 0;
            else if (r == 0x2e)
                U32(obj, 0x5cb) = 1;
            value = Sim_1004c325(c);
            speed += Sim_1004c362(c);
            armour += Sim_1004c3dd(c);
            sensors += Sim_1004c426(c);
            top += Sim_1004c3ad(sub, (int)c);
            weight += Sim_1004c4a1(sub, (int)c);
            x18 += Sim_1004c4d1(sub, (int)c);
            break;
        }
        case 8:
        case 9:
        case 10:
        case 11:
        case 12:
        case 13:
        case 14:
        case 15:
        case 16: {
            static const int32_t index[9] = {2, 1, 4, 0, 3, 8, 6, 7, 5};

            value = Sim_1004d330(sub);
            pof = index[kind - 8];
            break;
        }
        }
        if (kind > 0 && kind < 0x11) {
            e = BYTES(U32(obj, 0x88)) + (kind - 1) * 0x3c;
            U16(e, 0x36) = 0;
            U32(e, 0x38) = 0;
            FL(e, 0) = (float)size;
            FL(e, 4) = (float)size;
            if ((U32(obj, 0xbd3) & 2) != 0) {
                FL(e, 0) = (float)((double)FL(e, 0) + (double)GFL(0x1012b4c8));
                FL(e, 4) = (float)((double)FL(e, 4) + (double)GFL(0x1012b4c8));
            } else if ((U32(obj, 0xbd3) & 1) != 0) {
                FL(e, 0) = (float)((double)FL(e, 0) + (double)GFL(0x1012b4c4));
                FL(e, 4) = (float)((double)FL(e, 4) + (double)GFL(0x1012b4c4));
            }
            FL(e, 8) = (float)value;
            U16(e, 0x34) = U16(part, 0x10);
            for (j = 0; j < 0x14; j++)
                U16(e, 0xc + j * 2) = U16(part, 0x14 + j * 4);
        }
        if (kind > 7 && kind < 0x11) {
            const char *seg = SIM_PTR(const char, 0x10126d20 + pof * 9);
            uint32_t found, wk;
            int32_t mode;

            weight += Sim_1004d344(sub);
            x18 += (int32_t)W14P_ROW(sub, 0xaa);
            Str_Copy(name, seg);
            found = (uint32_t)(uintptr_t)Sim_1005ae58((void *)(uintptr_t)U32(U32(obj, 0), 0x48), name);
            U32(obj, 0x498 + mounts * 4) = found;
            if (U32(obj, 0x498 + mounts * 4) != 0)
                Sim_100aa4f0((void *)(uintptr_t)U32(obj, 0x498 + mounts * 4), 0x70);
            else
                W14P_PRINT(SIM_PTR(const char, 0x10144120), seg, base + 4);
            if (kind == 10)
                S32(obj, 0x5fb) = mounts;
            S32(slot, 0x11c) = mounts;
            wk = W14P_ROW(sub, 0xc6);
            mode = GS32(0x10124ed4u + Sim_1004d26d(c) * 0x98u);
            if (mode == 1) {
                U32(slot, 0x24) = 1;
                U32(slot, 0x28) = 5;
            } else if (mode == 2) {
                U32(slot, 0x24) = 1;
                U32(slot, 0x28) = 10;
            } else if (mode == 3) {
                U32(slot, 0x24) = 0xffffffffu;
                U32(slot, 0x28) = 10;
            } else {
                U32(slot, 0x24) = 0;
                U32(slot, 0x28) = 0;
            }
            U32(slot, 0) = 1;
            U32(slot, 4) = wk;
            S32(obj, 0x5ff) = Sim_1005b89b((int)U32(slot, 4), S32(obj, 0x5ff));
            U32(slot, 8) = 1;
            U32(slot, 0x118) = G32(0x101668e4);
            G32(0x101668e4) += 1;
            U32(slot, 0x114) = 0;
            U32(slot, 0x10) = 0;
            U32(slot, 0x14) = 0;
            S32(slot, 0x30) = Sim_1005b923((unsigned)kind);
            if (G32(0x1015ec58) == 0 || GS32(0x1014c99c) == -1)
                S32(slot, 0xc) = Sim_1004d27a((int)c);
            U32(slot, 0x34) = wk;
            U32(slot, 0x110) = (uint32_t)(uintptr_t)link;
            U32(slot, 0x108) = 0;
            U32(slot, 0x20) = 0;
            U16(link, 0) = U16(slot, 4);
            U16(link, 2) = (uint16_t)W14P_ROW(sub, 0xc2);
            U16(link, 6) = (uint16_t)mounts;
            U16(link, 8) = U16(link, 0);
            U32(link, 0xc) = G32(0x10145548u + U16(link, 0) * 0xd6u);
            U32(link, 0x14) = G32(0x1014554cu + U16(link, 0) * 0xd6u);
            U32(link, 0x10) = G32(0x10145550u + U16(link, 0) * 0xd6u);
            S32(link, 0x20) = mode;
            U32(slot, 0x20) += (uint32_t)(int32_t)S16(link, 2);
            S32(slot, 0x3c) = links;
            U32(slot, 0x38) += 1;
            link += 0x24;
            links++;
            if ((uintptr_t)SIM_PTR(uint8_t, 0x10145538) + wk * 0xd6u != 0)
                U32(slot, 8) = 1;
            else
                U32(slot, 8) = 0xffffffffu;
            Sim_NoteWeaponMount(obj, slot, kind);    /* round 22 fix: the part this weapon sits on */
            slot += 0x128;
            mounts++;
        }
    }
    S32(obj, 0x4f8) = mounts;
    weight = (int32_t)((uint32_t)weight + (uint32_t)total * (uint32_t)W14P_COLUMN(col2) + (uint32_t)W14P_COLUMN(col));
    Sim_1004bce5(&top, chassis, chassis_row, weight, x18);
    scale = Sim_1004bc65(weight, x18);
    speed += Sim_1004be11(weight, x18);
    e = BYTES(U32(obj, 0x88));
    for (i = 0; i < 0x11; i++, e += 0x3c) {
        uint32_t v;

        if ((U16(e, 0x36) & 0x800) != 0)
            continue;
        U16(e, 0x36) &= 0xff00;
        FL(e, 8) = (float)((double)FL(e, 8) * (double)scale);
        v = (uint32_t)Render_Ftol(((double)FL(e, 8) * (double)GFL(0x1012b4c0) +
                                   (double)FL(e, 0) * (double)GFL(0x1012b4c0)) /
                                  (double)GFL(0x1012b4c4));
        if (v > 0xffff)
            v = 0xffff;
        U32(e, 0x38) = v;
    }
    lim = 7.62939453125e-04f;
    if (GS32(0x1013abd8) <= -0x64)
        lim = (float)((double)lim * (double)GFL(0x1012b4c4));
    else if (GS32(0x1013abd8) < -0x1e)
        lim = (float)((double)lim * (double)GFL(0x1012b4cc));
    else if (GS32(0x1013abd8) > 0x32)
        lim = (float)((double)lim * (double)GFL(0x1012b4d0));
    FL(obj, 0x4ec) = lim;
    U32(obj, 0x538) = 0;
    S32(obj, 0x5e3) = Render_Ftol((double)FL(obj, 0x4d4));
    U32(obj, 0x5ef) = U32(obj, 0x4f8);
    U32(obj, 0x5cf) = 0;
    U32(obj, 0x5d3) = 0;
    U32(obj, 0x5d7) = 0;
    U32(obj, 0x5db) = 0;
    U32(obj, 0x5df) = 0;
    U32(obj, 0x5e7) = 0;
    U32(obj, 0x5eb) = 0;
    U32(obj, 0x5f3) = 0;
    U16(U32(obj, 0), 0x1ac) = 0;
    if (speed < 1)
        speed = 1;
    else if (speed > top)
        speed = top;
    U32(obj, 0x60b) = (U32(obj, 0xbd3) & 0x20) != 0 ? 0x5a : 0xb5;
    FL(obj, 0x4d4) = (float)((double)speed * (double)GFL(0x1012b4d4));
    S32(obj, 0xbd7) = armour;
    S32(obj, 0xbdb) = class_;
    S32(obj, 0xbdf) = sensors;
    S32(obj, 0xbe3) = cockpit;
    U32(obj, 0xbe7) = 1;
    U32(obj, 0xc35) = c35;
    U32(obj, 0xbb3) = bb3;
    U32(obj, 0xbaf) = baf;
    U32(obj, 0xbab) = bab;
    U32(obj, 0xbbf) = bbf;
    U32(obj, 0xbb7) = bb7;
    U32(obj, 0xbbb) = flags;
    U32(obj, 0xbc3) = bc3;
    U32(obj, 0xbc7) = bc7;
    U32(obj, 0xc39) = 0;
    U32(obj, 0xc3d) = 0;
    U32(obj, 0xc4d) = 0;
    if (jets == 0) {
        S32(obj, 0x510) = -2;
        U32(obj, 0x514) = 0;
        U32(obj, 0x540) = 0;
        U32(obj, 0x518) = 0;
    } else {
        U32(obj, 0x510) = 0x2d4;
        U32(obj, 0x514) = 10;
        FL(obj, 0x540) = (float)((double)GFL(0x1012b4d8) * (double)jump);
        U32(obj, 0x518) = 0;
    }
    Sim_1005b7e0(obj);
    return 1;
}

/* The system slot of an old-format part kind (0x1005cd4a): -1 none, 0..15 the entry +0x88, 7..15 a weapon mount whose
 * POF segment name is copied (the names are 8 characters). */
static int32_t W14P_OldSlot(int32_t kind, char *name)
{
    static const struct {
        int32_t kind, slot;
        uint32_t seg;
    } mounts[9] = {{0x21, 0xd, 0x10144198}, {0x22, 0xf, 0x101441b0}, {0x23, 0xc, 0x1014418c},
                   {0x24, 0xe, 0x101441a4}, {0x26, 10, 0x10144174},  {0x27, 8, 0x1014415c},
                   {0x29, 0xb, 0x10144180}, {0x2a, 9, 0x10144168},   {0x2c, 7, 0x10144150}};
    unsigned i;

    if (kind == 1 || kind == 2)
        return 1;
    if (kind == 3)
        return 0;
    if (kind >= 5 && kind <= 0xc)
        return 3;
    if (kind >= 0xd && kind <= 0x14)
        return 2;
    if (kind >= 0x16 && kind <= 0x1a)
        return 5;
    if (kind >= 0x1b && kind <= 0x1f)
        return 4;
    if (kind == 0x20)
        return 6;
    for (i = 0; i < 9; i++) {
        if (mounts[i].kind == kind) {
            Str_Copy(name, SIM_PTR(const char, mounts[i].seg));
            return mounts[i].slot;
        }
    }
    return -1;
}

/* Kind 1 (the chassis of the old format): its sub-kind's weight, weight sum, armour and jump factor. */
static void W14P_OldChassis(int32_t sub, int32_t *weight, int32_t *wsum, int32_t *armour, float *jump)
{
    static const int32_t add64[6] = {3, 12, 9, 10, 8, 8}, add18[6] = {0, -10, -15, -14, -25, -30};
    static const int32_t addb0[6] = {0, 1, 2, 3, 5, 10};
    static const uint32_t f[6] = {0, 0x3e800000u, 0x3e90e560u, 0x3ea1cac1u, 0x3eb33333u, 0};
    union {
        uint32_t u;
        float f;
    } v;

    if (sub < 0 || sub > 5) {
        *weight += 3;
        *jump = 0.0f;
        return;
    }
    *weight += add64[sub];
    *wsum += add18[sub];
    *armour += addb0[sub];
    v.u = f[sub];
    *jump = v.f;
}

/* Kind 3 (cockpit): the cockpit total by sub-kind and the part's words; word 7 sets the flag +0xc35, word 0xe trades
 * 2 cockpit for 5 sensors. */
static void W14P_OldCockpit(const uint8_t *part, int32_t *cockpit, int32_t *sensors, uint32_t *c35)
{
    int32_t j;

    switch (S32(part, 4)) {
    case 3:
        *cockpit += 6;
        break;
    case 4:
        *cockpit += 7;
        break;
    case 5:
        *cockpit += 2;
        break;
    case 6:
    case 7:
    case 8:
    case 9:
        *cockpit += 3;
        break;
    }
    for (j = 0; j < 0x14; j++) {
        switch (S32(part, 0x14 + j * 4)) {
        case 1:
            *cockpit += 2;
            break;
        case 2:
            *cockpit += 1;
            break;
        case 5:
        case 0xd:
            *cockpit += 4;
            break;
        case 6:
            *cockpit += 3;
            break;
        case 7:
            *c35 = 1;
            break;
        case 0xe:
            *cockpit -= 2;
            *sensors += 5;
            break;
        }
    }
}

/* Kinds 0xd..0x14 (armour): armour by sub-kind and words, and the extra value of the system entry. */
static void W14P_OldArmour(const uint8_t *part, int32_t *armour, int32_t *extra)
{
    int32_t j, sub = S32(part, 4);

    if (sub == 0) {
        *armour += 1;
    } else if (sub == 1) {
        *armour += 2;
        *extra = 0x32;
    } else {
        *armour += 3;
    }
    for (j = 0; j < 0x14; j++) {
        switch (S32(part, 0x14 + j * 4)) {
        case 2:
        case 7:
        case 0xc:
        case 0x11:
        case 4:
        case 9:
        case 0xe:
        case 0x13:
        case 0x15:
        case 0x17:
            *armour += 1;
            break;
        case 5:
        case 10:
        case 0xf:
        case 0x14:
        case 0x16:
        case 0x18:
            *armour += 2;
            break;
        case 0x19:
            *extra += 0x32;
            break;
        case 0x1a:
            *extra += 100;
            break;
        case 0x1b:
            *extra += 0x96;
            break;
        }
    }
}

/* Kinds 0x1b..0x1f (engine): weight sum and weight by sub-kind and words, and the extra value. */
static void W14P_OldEngine(const uint8_t *part, int32_t *wsum, int32_t *weight, int32_t *extra)
{
    int32_t j, sub = S32(part, 4);

    if (sub == 0) {
        *wsum += 1;
        *weight += 2;
    } else if (sub == 1) {
        *wsum += 2;
        *weight += 3;
        *extra = 0x32;
    } else if (sub == 2) {
        *wsum += 3;
        *weight += 6;
        *extra = 0x32;
    }
    for (j = 0; j < 0x14; j++) {
        switch (S32(part, 0x14 + j * 4)) {
        case 1:
        case 6:
        case 0xb:
        case 0x10:
            *extra -= 0x19;
            break;
        case 2:
        case 7:
        case 0xc:
        case 0x11:
            *wsum += 1;
            *weight += 1;
            break;
        case 4:
        case 9:
        case 0xe:
        case 0x13:
            *wsum += 1;
            *weight += 2;
            break;
        case 5:
        case 10:
        case 0xf:
        case 0x14:
            *wsum += 2;
            *weight += 1;
            break;
        case 0x15:
        case 0x16:
        case 0x17:
        case 0x18:
            *extra += 0x32;
            break;
        case 0x19:
            *extra += 100;
            break;
        case 0x1a:
            *extra += 200;
            *wsum -= 1;
            *weight -= 1;
            break;
        case 0x1b:
            *extra += 300;
            *wsum -= 1;
            *weight -= 1;
            break;
        }
    }
}

typedef double(__cdecl *W14pSqrtFn)(double);

// FUNCTION: HGSIM 0x1005cd4a
int __cdecl Sim_1005cd4a(uint8_t *obj, uint8_t *design)
{
    /* the design header +0x8c..+0xc7 (kind, name +4, part count +0x24, flags +0x2c..+0x38), copied as the original
     * does; the name is a NUL-terminated design name */
    uint8_t hdr[0x3c + 4] = {0};
    int32_t parts, i, j, links = 0, mounts = 0, weight = 0, wsum = 0, armour = 0, cls = 0, sensors = 0, cockpit = 0;
    int32_t jets = 0, slot_code, extra;
    uint32_t c35 = 0;
    float jump = 0.0f, lim;
    uint8_t *slot, *link, *part, *e;
    char name[12];

    if (U32(design, 0x8c) == 2)
        return Sim_1005bb7a(obj, design);
    U8(obj, 0x594) = 0;
    U16(obj, 0x562) = 0;
    U16(obj, 0x560) = 0;
    U32(obj, 0x5ff) = 0;
    for (i = 0; i < 0x3c; i++)
        hdr[i] = design[0x8c + i];
    parts = S32(hdr, 0x24);
    part = BYTES(U32(design, 0xc8));
    for (i = 0; i < 10; i++)
        U32(obj, 0x498 + i * 4) = 0;
    slot = BYTES(U32(obj, 0x84));
    link = BYTES(U32(obj, 0x8c));
    for (i = 0; i < parts; i++, part += 0x6c) {
        int32_t kind = S32(part, 0);

        extra = 0;
        c35 = 0;
        if (kind == 1) {
            sensors = S32(part, 0x64);
            cls = S32(part, 4);
            W14P_OldChassis(cls, &weight, &wsum, &armour, &jump);
            jets = 0;
            for (j = 0; j < 0x14; j++)
                if (S32(part, 0x14 + j * 4) == 1)
                    jets = 1;
        } else if (kind == 3) {
            W14P_OldCockpit(part, &cockpit, &sensors, &c35);
        } else if (kind >= 0xd && kind <= 0x14) {
            W14P_OldArmour(part, &armour, &extra);
        } else if (kind >= 0x1b && kind <= 0x1f) {
            W14P_OldEngine(part, &wsum, &weight, &extra);
        } else if (kind == 0x20) {
            U32(obj, 0x5cb) = U32(part, 4);
            wsum += Render_Ftol(hg_sqrt(
                (double)(S32(part, 0x64) == 0 ? S32(part, 0xc) : S32(part, 0x64))));
        }
        slot_code = W14P_OldSlot(kind, name);
        if (slot_code > -1 && slot_code < 0x10) {
            e = BYTES(U32(obj, 0x88)) + slot_code * 0x3c;
            U16(e, 0x36) = 0;
            U32(e, 0x38) = 0;
            FL(e, 0) = (float)S32(part, 8);
            FL(e, 4) = (float)S32(part, 8);
            if ((U32(obj, 0xbd3) & 2) != 0) {
                FL(e, 0) = (float)((double)FL(e, 0) + (double)GFL(0x1012b4c8));
                FL(e, 4) = (float)((double)FL(e, 4) + (double)GFL(0x1012b4c8));
            } else if ((U32(obj, 0xbd3) & 1) != 0) {
                FL(e, 0) = (float)((double)FL(e, 0) + (double)GFL(0x1012b4c4));
                FL(e, 4) = (float)((double)FL(e, 4) + (double)GFL(0x1012b4c4));
            }
            FL(e, 8) = (float)((double)S32(part, 0xc) + (double)extra);
            U16(e, 0x34) = U16(part, 0x10);
            for (j = 0; j < 0x14; j++)
                U16(e, 0xc + j * 2) = U16(part, 0x14 + j * 4);
        }
        if (slot_code > 6) {
            uint32_t wk, s;
            int32_t k, mode;

            for (k = 0; k < 10 && U32(obj, 0x498 + k * 4) != 0; k++)
                ;
            U32(obj, 0x498 + k * 4) =
                (uint32_t)(uintptr_t)Sim_1005ae58((void *)(uintptr_t)U32(U32(obj, 0), 0x48), name);
            if (U32(obj, 0x498 + k * 4) != 0)
                Sim_100aa4f0((void *)(uintptr_t)U32(obj, 0x498 + k * 4), 0x70);
            if (slot_code == 9)
                S32(obj, 0x5fb) = k;
            S32(slot, 0x11c) = k;
            wk = U32(part, 4);
            mode = S32(part, 0x64);
            if (mode == 1) {
                U32(slot, 0x24) = 1;
                U32(slot, 0x28) = 5;
            } else if (mode == 2) {
                U32(slot, 0x24) = 1;
                U32(slot, 0x28) = 10;
            } else if (mode == 3) {
                U32(slot, 0x24) = 0xffffffffu;
                U32(slot, 0x28) = 10;
            } else {
                U32(slot, 0x24) = 0;
                U32(slot, 0x28) = 0;
            }
            U32(slot, 0) = 1;
            U32(slot, 4) = wk;
            if (Crt_StrStr((const char *)hdr + 4, SIM_PTR(const char, 0x101441bc)) != 0) {
                if (U32(slot, 4) == 0)
                    U32(obj, 0x5ff) = 0x100;
                if (U32(slot, 4) == 0x29)
                    U32(obj, 0x5ff) = 0x200;
            } else {
                S32(obj, 0x5ff) = Sim_1005b89b((int)U32(slot, 4), S32(obj, 0x5ff));
            }
            U32(slot, 8) = 1;
            U32(slot, 0x118) = G32(0x101668e4);
            G32(0x101668e4) += 1;
            U32(slot, 0x114) = 0;
            U32(slot, 0x10) = 0;
            U32(slot, 0x14) = 0;
            S32(slot, 0x30) = slot_code;
            if (G32(0x1015ec58) == 0 || GS32(0x1014c99c) == -1) {
                U32(slot, 0xc) = 0;
                for (j = 0; j < S32(part, 0x10); j++) {
                    if (U32(part, 0x14 + j * 4) != 0) {
                        S32(slot, 0xc) = j;
                        break;
                    }
                }
            }
            U32(slot, 0x34) = U32(part, 0);
            U32(slot, 0x108) = 0;
            U32(slot, 0x110) = (uint32_t)(uintptr_t)link;
            s = hdr[4] == 's' || hdr[4] == 'S';
            U32(slot, 0x20) = GS32(0x10145538u + s * 4u + wk * 0xd6u) == -1 ? 0xffffffffu : 0;
            for (j = 0; j < S32(part, 0x68); j++) {
                U16(link, 0) = U16(slot, 4);
                U16(link, 2) = (uint16_t)(G32(0x10145538u + s * 4u + U16(link, 0) * 0xd6u) *
                                          G32(0x10145534u + U16(link, 0) * 0xd6u));
                U16(link, 6) = (uint16_t)mounts;
                U16(link, 8) = U16(link, 0);
                U32(link, 0xc) = G32(0x10145548u + U16(link, 0) * 0xd6u);
                U32(link, 0x14) = G32(0x1014554cu + U16(link, 0) * 0xd6u);
                U32(link, 0x10) = G32(0x10145550u + U16(link, 0) * 0xd6u);
                U32(link, 0x20) = U32(part, 0x64);
                U32(slot, 0x20) += (uint32_t)(int32_t)S16(link, 2);
                S32(slot, 0x3c + j * 4) = links;
                U32(slot, 0x38) += 1;
                link += 0x24;
                links++;
            }
            if ((uintptr_t)SIM_PTR(uint8_t, 0x10145538) + wk * 0xd6u != 0)
                U32(slot, 8) = 1;
            else
                U32(slot, 8) = 0xffffffffu;
            Sim_ForgetWeaponMount(slot);             /* round 22 fix: these codes already match */
            slot += 0x128;
            mounts++;
        }
    }
    S32(obj, 0x4f8) = mounts;
    e = BYTES(U32(obj, 0x88));
    for (i = 0; i < 0x11; i++, e += 0x3c) {
        uint32_t v;

        if ((U16(e, 0x36) & 0x800) != 0)
            continue;
        U16(e, 0x36) &= 0xff00;
        v = (uint32_t)Render_Ftol(((double)FL(e, 8) * (double)GFL(0x1012b4c0) +
                                   (double)FL(e, 0) * (double)GFL(0x1012b4c0)) /
                                  (double)GFL(0x1012b4c4));
        if (v > 0xffff)
            v = 0xffff;
        U32(e, 0x38) = v;
    }
    lim = 7.62939453125e-04f;
    if (GS32(0x1013abd8) <= -0x64)
        lim = (float)((double)lim * (double)GFL(0x1012b4c4));
    else if (GS32(0x1013abd8) < -0x1e)
        lim = (float)((double)lim * (double)GFL(0x1012b4cc));
    else if (GS32(0x1013abd8) > 0x32)
        lim = (float)((double)lim * (double)GFL(0x1012b4d0));
    FL(obj, 0x4ec) = lim;
    U32(obj, 0x538) = 0;
    S32(obj, 0x5e3) = Render_Ftol((double)FL(obj, 0x4d4));
    U32(obj, 0x5ef) = U32(obj, 0x4f8);
    U32(obj, 0x5cf) = 0;
    U32(obj, 0x5d3) = 0;
    U32(obj, 0x5d7) = 0;
    U32(obj, 0x5db) = 0;
    U32(obj, 0x5df) = 0;
    U32(obj, 0x5e7) = 0;
    U32(obj, 0x5eb) = 0;
    U32(obj, 0x5f3) = 0;
    U16(U32(obj, 0), 0x1ac) = 0;
    S32(obj, 0xbd7) = armour;
    S32(obj, 0xbdb) = cls;
    S32(obj, 0xbdf) = sensors;
    S32(obj, 0xbe3) = cockpit;
    U32(obj, 0xbe7) = 1;
    U32(obj, 0xbcb) = U32(hdr, 0x30);
    U32(obj, 0xbcf) = U32(hdr, 0x34);
    U32(obj, 0xbd3) = U32(hdr, 0x38);
    U32(obj, 0x60b) = 0xb5;
    U32(obj, 0x613) = U32(hdr, 0x2c);
    U32(obj, 0xc15) = 0;
    U32(obj, 0xc35) = c35;
    U32(obj, 0xc39) = 0;
    U32(obj, 0xc3d) = 0;
    if (jets == 0) {
        S32(obj, 0x510) = -2;
        U32(obj, 0x514) = 0;
        U32(obj, 0x540) = 0;
        U32(obj, 0x518) = 0;
    } else {
        U32(obj, 0x510) = 0x2d4;
        U32(obj, 0x514) = 10;
        FL(obj, 0x540) = (float)((double)GFL(0x1012b4d8) * (double)jump);
        U32(obj, 0x518) = 0;
    }
    Sim_1005b7e0(obj);
    Sim_1005e295(obj, weight, wsum);
    return 1;
}
