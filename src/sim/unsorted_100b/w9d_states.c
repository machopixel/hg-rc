#include "w9d.h"
#include "../unsorted_1001/w2e.h"
#include "../unsorted_1003/fns.h"
#include "../unsorted_100c/fns.h"
#include "../unsorted_100c/w2e.h"
#include "../unsorted_100c/w5a.h"
#include "../util/access.h"
#include "../util/util.h"

/* Hands the selected weapon back when bit 0x100 of the word +0x1a8 is set. */
static void W9d_ReleaseWeapon(void *obj)
{
    if ((U16(obj, 0x1a8) & 0x100) != 0) {
        Sim_100ca417(U32(obj, 4), U16(obj, 0x1a8));
        U16(obj, 0x1a8) = U16(obj, 0x1aa);
    }
}

static void W9d_ClearTarget(void *obj)
{
    if (S16(obj, 0x22c) == -1 || G32(0x1017e2a8) == 1 || G32(0x1017e2ac) == 1 || G32(0x1017e2b0) == 1) {
        U16(obj, 0x1f8) = 0xffff;
        U16(obj, 0x1fa) = 0xffff;
        U32(obj, 0x22e) = 0xffffffffu;
        U32(obj, 0x23a) = 0xffffffffu;
    }
}

static void W9d_LeaveEntry(void *obj)
{
    if (S32(obj, 0x23a) >= 0) {
        unsigned char *entry = Sim_10033c42((int)U32(obj, 0x23a));

        if (entry != 0)
            Sim_100335cf(entry, (const unsigned char *)obj);
    }
    Sim_100c48c9(obj);
}

// FUNCTION: HGSIM 0x100bfa33
void __cdecl Sim_100bfa33(void *obj)
{
    uint32_t state = (uint32_t)(int32_t)S16(obj, 0x1f8);
    uint32_t rec = U32(obj, 0x54);

    switch (state) {
    case 1:
        Sim_1001570f(obj);
        if ((U16(obj, 0x1aa) & 0x200) != 0) {
            int32_t half = S32(obj, 0x218) / 2;
            uint32_t unit = PTRTAB(U16(obj, 0x1aa) & 0xff);

            U8(unit, 0x270 + half) = (uint8_t)(U8(unit, 0x270 + half) - 1);
        }
        break;
    case 4:
        U32(obj, 0x204) = 0;
        U32(obj, 0x218) = 0;
        break;
    case 5:
        Sim_100c274a(obj, 0);
        U8(rec, 0x21) = 0;
        U8(rec, 0x22) = 0;
        break;
    case 7:
    case 0xc:
    case 0xd:
        Sim_1001570f(obj);
        break;
    case 8:
    case 0xa:
        U8(rec, 0x59) = 0;
        break;
    case 9:
        Sim_100c274a(obj, 0);
        break;
    case 0xb:
        U8(rec, 0x21) = 0;
        U8(rec, 0x22) = 0;
        U8(rec, 0x1f) = 0;
        U8(rec, 0x20) = 0;
        Sim_100c274a(obj, 0);
        break;
    case 0xe:
        W9d_ReleaseWeapon(obj);
        U32(obj, 0x218) = 0;
        U32(obj, 0x21c) = 0;
        U32(obj, 0x220) = 0;
        W9d_ClearTarget(obj);
        break;
    case 0x10:
        W9d_LeaveEntry(obj);
        break;
    case 0x11:
        if (S16(obj, 0x22c) == -1) {
            U16(obj, 0x1fa) = 0xffff;
            U16(obj, 0x1f8) = 0xffff;
        }
        W9d_LeaveEntry(obj);
        U32(obj, 0x218) = 0;
        U32(obj, 0x21c) = 0;
        U32(obj, 0x220) = 0;
        U32(obj, 0x224) = 0;
        U32(obj, 0x228) = 0;
        U32(obj, 0x204) = 0;
        Sim_100c48fa(obj, 2);
        break;
    case 0x13:
        W9d_ReleaseWeapon(obj);
        U32(obj, 0x218) = 0;
        break;
    case 0x14:
        U32(obj, 0x204) = 0;
        U32(obj, 0x218) = 0;
        U32(obj, 0x21c) = 0;
        break;
    case 0x15:
        U32(obj, 0x218) = 0;
        U32(obj, 0x204) = 0;
        U32(rec, 8) = 0;
        U8(rec, 0x59) = 0;
        U8(rec, 0x56) = 0;
        break;
    case 0x16:
    case 0x17:
        W9d_ReleaseWeapon(obj);
        U32(obj, 0x218) = 0;
        U32(obj, 0x21c) = 0;
        U32(obj, 0x220) = 0;
        U32(obj, 0x224) = 0;
        W9d_ClearTarget(obj);
        break;
    case 0x1a:
        U32(obj, 0x218) = 0;
        Sim_100c48c9(obj);
        break;
    case 0x1b:
        U32(obj, 0x218) = 0;
        Sim_100c48fa(obj, 2);
        break;
    default:
        break;
    }
    Sim_100bf9a1(obj);
}
