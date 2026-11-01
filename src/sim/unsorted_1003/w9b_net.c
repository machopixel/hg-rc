#include "w9b.h"
#include "fns.h"
#include "w3p.h"
#include "w5a.h"
#include "w5c.h"
#include "../core/w2a.h"
#include "../core/w2b.h"
#include "../unsorted_1004/w2b.h"
#include "../unsorted_1004/w2h.h"
#include "../unsorted_1004/w2i.h"
#include "../unsorted_1004/w2j.h"
#include "../unsorted_1004/w3p.h"
#include "../unsorted_1004/w5a.h"
#include "../unsorted_1004/w5y.h"
#include "../unsorted_1004/w6e.h"
#include "../unsorted_1004/w7m.h"
#include "../unsorted_1004/w8u.h"
#include "../unsorted_1004/w9b.h"
#include "../unsorted_1006/w8w.h"
#include "../unsorted_1006/w9b.h"
#include "../unsorted_1007/w2e.h"
#include "../unsorted_1007/w3p.h"
#include "../unsorted_1007/w3s.h"
#include "../unsorted_100a/w5b.h"
#include "../unsorted_100a/w5x.h"
#include "../unsorted_100a/w5y.h"
#include "../unsorted_100a/w5z.h"
#include "../unsorted_100f/w2e.h"
#include "../unsorted_100f/w6i.h"
#include "../unsorted_100f/w8z.h"
#include "../util/access.h"
#include "../util/w3o.h"

#define ROOT G32(0x1015d004)
#define SELF G32(0x1017b8c8)
#define RECV BYTES(G32(0x1015d008))
#define LAST_SEEN(i) (SIM_PTR(u32a, 0x1015db00)[(i)])
#define TEXT(addr) SIM_PTR(const char, addr)
#define TEXT_AT(addr) ((const char *)(uintptr_t)G32(addr))

static int W9b_Tag(const char *tag, uint32_t addr)
{
    return Str_CompareNoCase(tag, TEXT(addr)) == 0;
}

// FUNCTION: HGSIM 0x1003e291
int __cdecl Net_ProcessMessages(void)
{
    int all_ready = 1;
    uint32_t extra;
    char tag[3];
    int who;
    uint16_t type;
    uint8_t *entry;
    char text[0x50];
    int i;

    if (G32(0x1017b8b8) != 0 && G32(0x1015ecdc) == 0 && Sim_100420a0() != 0) {
        G32(0x1015ecdc) = 1;
        G32(0x1017b8bc) += 1;
    }
    if (G32(0x1015eccc) != 1)
        return 1;
    if (GS32(0x1015ec58) <= 1)
        return 1;
    *SIM_PTR(u16a, 0x1015ec60) = 0;
    G32(0x1015dc7c) = (uint32_t)Sim_1002ac1b();     /* once; the loop below starts at the receive */
    for (;;) {
        type = (uint16_t)Sim_1007e984(&extra);
        if (type == 0xfa01)
            break;
        g_w34NetOver = G32(0x1015f1fc) != 0;        /* 1.2: the receive guard, from the game-over flag */
        G32(0x1015ec6c) += 1;
        tag[0] = (char)RECV[0];
        tag[1] = (char)RECV[1];
        tag[2] = 0;
        if (type == 0 && W9b_Tag(tag, 0x1013acb4)) {
            /* "eD": a player has left */
            type = *(u16a *)(RECV + 2);
            who = (int)Sim_1003db56((int16_t)type);
            if (who < 0)
                continue;
            U16(BYTES(U32(ROOT, 0x16)), (uint32_t)Sim_1003dbc0((uint32_t)who) * 2) = 0xfa01;
            entry = BYTES(PTRTAB(who));
            if (entry == 0)
                continue;
            if (G32(0x1015f1fc) == 0) {
                /* (the original tests ([0x1017b94c] | 0x80000000) != 0 here, which always holds) */
                Sim_1006abf6(0xffffffffu, BYTES(U32(entry, 0x28)), 2);
                Score_RecordKill(-2, BYTES(U32(entry, 0x28)));
                U32(entry, 0x160) = 1;
            }
            Sim_100421b0(type);
            Crt_SPrintf(text, TEXT(0x1013acb8), (const char *)entry + 0x124, TEXT_AT(0x101c4a00));
            Sim_1007df6c(text, TEXT_AT(0x109b98c0), 0x712, 0x50);
            break;
        }
        who = (int)Sim_1003db56((int16_t)type);
        if ((uint32_t)who == SELF || who < 0)
            continue;
        entry = BYTES(PTRTAB(who));
        if (entry == 0)
            continue;
        LAST_SEEN(who) = G32(0x1015dc7c);
        if (W9b_Tag(tag, 0x1013acc0)) {                             /* "DA" */
            W9B3_CALL_1003f3f8(RECV, who, extra);
            if (S16(entry, 0x1c) & 0x4000) {
                Sim_100aa39f(BYTES(U32(entry, 0x48)));
                Sim_100aa454(BYTES(U32(entry, 0x48)));
                Sim_100aa799(BYTES(U32(entry, 0x48)));
                U16(entry, 0x1c) = (uint16_t)(U16(entry, 0x1c) & 0xb7ff);
                U32(U32(entry, 0x28), 0x4f0) = 2;
                if (U32(entry, 0x14) != 3)
                    U32(entry, 0x14) = 1;
                Sim_100421b0(0xfa01);
            }
        } else if (W9b_Tag(tag, 0x1013acc4)) {                      /* "WE" */
            if (extra == 5)
                Sim_1004044c(RECV);
        } else if (W9b_Tag(tag, 0x1013acc8)) {                      /* "DM" */
            if (extra == 0x16)
                Sim_10040191(RECV);
        } else if (W9b_Tag(tag, 0x1013accc)) {                      /* "KI" */
            if (extra == 0xc)                       /* 1.2: 10 players */
                Sim_1004158c(RECV, who);
        } else if (Str_CompareNoCase(tag, "AK") == 0) {            /* 1.2's new message */
            if (extra == 0xe)
                Sim12_10035348(RECV);
        } else if (W9b_Tag(tag, 0x1013acd0)) {                      /* "CO" */
            if (extra == 0x1a)
                Sim_1004170e(RECV, who);
        } else if (W9b_Tag(tag, 0x1013acd4)) {                      /* "GO" */
            if (extra == 2)
                Sim_100418e3((int)(uintptr_t)RECV, who);
        } else if (W9b_Tag(tag, 0x1013acd8)) {                      /* "SN" */
            if (extra == G32(0x1015d08c))
                Sim_10041b2d(RECV, who);
        } else if (W9b_Tag(tag, 0x1013acdc)) {                      /* "SU" */
            if (extra == 2)
                Sim_10041c4c((uint32_t)(uintptr_t)RECV, who);
        } else if (W9b_Tag(tag, 0x1013ace0)) {                      /* "EG" */
            if (extra == 2)
                Sim_10041a16();
        } else if (W9b_Tag(tag, 0x1013ace4)) {                      /* "SS" */
            if (extra == 2) {
                const u16a *word = (const u16a *)(RECV + 2);

                if (G32(0x1015f1fc) == 0) {
                    /* (the same always-true test of [0x1017b94c] as above) */
                    Sim_1006abf6(0xfffffffeu, BYTES(U32(entry, 0x28)), 2);
                    Score_RecordKill(-2, BYTES(U32(entry, 0x28)));
                }
                SIM_PTR(u32a, 0x1015db40)[who] = 1;
                Sim_100421b0(*word);
                Crt_SPrintf(text, TEXT(0x1013ace8), (const char *)entry + 0x124, TEXT_AT(0x101c4a00));
                Sim_1007df6c(text, TEXT_AT(0x109b98c0), 0x712, 0x50);
                break;
            }
        } else if (W9b_Tag(tag, 0x1013acf0)) {                      /* "CH" */
            Sim_1003ff72(RECV, who);
        } else if (W9b_Tag(tag, 0x1013acf4)) {                      /* "LA" */
        } else if (W9b_Tag(tag, 0x1013acf8)) {                      /* "CA" */
            Sim_100405e6(RECV);
        } else if (W9b_Tag(tag, 0x1013acfc)) {                      /* "TC" */
            Sim_1004084a(RECV);
        } else if (W9b_Tag(tag, 0x1013ad00)) {                      /* "AI" */
            Sim_1004094e(RECV, who);
        } else if (W9b_Tag(tag, 0x1013ad04)) {                      /* "DS" */
            Sim_10040a37(RECV);
        } else if (W9b_Tag(tag, 0x1013ad08)) {                      /* "RG" */
            W9B3_CALL_10040af8(RECV);
        } else if (W9b_Tag(tag, 0x1013ad0c)) {                      /* "NR" */
            Sim_10040f2d(RECV);
        } else if (W9b_Tag(tag, 0x1013ad10)) {                      /* "OR" */
            Sim_100410ff(RECV);
        } else if (W9b_Tag(tag, 0x1013ad14)) {                      /* "PU" */
            Sim_10041247(RECV);
        } else if (W9b_Tag(tag, 0x1013ad18)) {                      /* "DR" */
            Sim_1004131b(RECV);
        } else if (W9b_Tag(tag, 0x1013ad1c)) {                      /* "KA" */
        } else {
            G32(0x1015ec70) += 1;
        }
    }
    if (*SIM_PTR(s16a, 0x1015ec7c) != 0) {
        if ((G32(0x1017b94c) & 1) == 0) {
            G32(0x1017b94c) |= 1;
        } else {
            if ((int32_t)(G32(0x1015dc7c) - G32(0x1015ece0)) >= 0) {
                for (i = 0; i < 8; i++) {
                    uint8_t *e;

                    if ((uint32_t)i == SELF || PTRTAB(i) == 0)
                        continue;
                    e = BYTES(PTRTAB(i));
                    if (U32(e, 0x14) == 3 && S32(e, 0xc) >= 0)
                        LAST_SEEN(i) = LAST_SEEN(S32(e, 0xc));
                    if (S16(e, 0x1c) & 0x4000)
                        continue;
                    if (U32(e, 0x14) != 1) {
                        if (U32(e, 0x14) != 3 || U32(e, 0xc) == SELF)
                            continue;
                    }
                    if ((int32_t)(G32(0x1015dc7c) - LAST_SEEN(i)) <= 0x38e)
                        continue;
                    if (U32(e, 0x14) == 3) {
                        U32(e, 0xc) = 0xffffffffu;
                        U32(e, 0x18) = 0;
                        continue;
                    }
                    if (U32(e, 0x48) != 0) {
                        Sim_100aa351(BYTES(U32(e, 0x48)));
                        Sim_100aa406(BYTES(U32(e, 0x48)));
                    }
                    U16(e, 0x1c) = (uint16_t)(U16(e, 0x1c) | 0x4800);
                    U32(e, 0x160) = 1;
                    Sim_100421b0((uint16_t)i);
                }
                G32(0x1015ece0) = G32(0x1015dc7c) + G32(0x1013ac9c);
                Sim_10040376((int)SELF);
                Net_SendStateMessage((int)SELF, 0);
                G32(0x1015ec68) += 1;
                if (G32(0x1015ec68) & 1)
                    Sim_10041a47();
                Net_UpdatePlayers();
            }
            if (G32(0x1015ecd0) == 1) {
                if (G32(0x1013aca0) != U8(ROOT, 0x137) || G32(0x1013aca4) != U8(ROOT, 0x138))
                    Sim_10040eb3();
            }
            for (i = 0; i < GS32(0x1015dc64); i++) {
                uint8_t *e = BYTES(PTRTAB(i));
                uint16_t id;

                if (U32(e, 0x14) != 0 && U32(e, 0x14) != 1)
                    continue;
                id = (uint16_t)(U8(e, 4) | 0x200);
                if (all_ready != 0 && Sim_10042908((const unsigned char *)&id) != 0)
                    all_ready = 1;
                else
                    all_ready = 0;
            }
            if (all_ready != 0 && G32(0x1015f1fc) == 0) {
                G32(0x1015f1fc) = 1;
                G32(0x1015f1e4) = 0;
                G32(0x1015f1d0) = 0;
                if (G32(0x1015ec58) != 0)
                    Sim_100419e9();
            }
        }
    } else {
        if ((int32_t)(G32(0x1015dc7c) - G32(0x1015ece4)) >= 0) {
            for (i = 0; i < GS32(0x1015dc64); i++) {
                uint8_t *e = BYTES(PTRTAB(i));

                if (e != 0 && U32(e, 0x14) == 3 && U32(e, 0xc) == SELF)
                    Sim_100408bd(i);
            }
            G32(0x1015ece4) = G32(0x1015dc7c) + G32(0x1013ac9c);
        }
        if (G32(0x1015ec54) != 0) {
            if (G32(0x1015ecd0) == 1) {
                GBYTES(0x1015d090)[SELF] = 1;
                Sim_1004185b();
            } else {
                Sim_10041888();
            }
        } else {
            Sim_10041997();
        }
    }
    if (*SIM_PTR(s16a, 0x1015ec60) == 0 && G32(0x1015dc84) == 2)
        G32(0x1015dc78) += (uint32_t)Sim_1002abb0();
    if (G32(0x1015dc84) == 2)
        Sim_1002abc3();
    return 1;
}

/* trunc(x * 65536 + 0.5) with the floats at 0x10111508 / 0x1011150c, computed in double like the x87 did. */
static int32_t W9b_Fixed(float x)
{
    return Render_Ftol((double)x * (double)GFL(0x10111508) + (double)GFL(0x1011150c));
}

// FUNCTION: HGSIM 0x1003eebc
void __cdecl Net_SendStateMessage(int index, int guaranteed)
{
    uint8_t *msg = BYTES(G32(0x1015ec88));
    uint8_t *entry;
    uint8_t *block;
    uint32_t kind = 0;

    Mem_Move(msg, GBYTES(0x1013ad20), 2);
    entry = BYTES(PTRTAB(index));
    if (entry == 0)
        return;
    if (U32(entry, 0x14) != 0 && U32(entry, 0x14) != 3)
        return;
    block = BYTES(U32(entry, 0x28));
    U16(msg, 7) = 0;
    msg[9] = 0;
    msg[2] = (uint8_t)index;
    U16(msg, 3) = (uint16_t)U32(entry, 0xf0);
    if ((U32(entry, 0xf0) & 0xf00) == 0x400) {
        kind = U32(entry, 0xf0) & 0xff;
        if (S16(GBYTES(0x109e9ba0), kind * 0x50) & 0x20) {
            uint32_t bit = (uint32_t)1 << (U32(entry, 8) & 31);

            if (((uint32_t)(int32_t)S16(GBYTES(0x109e9ba6), kind * 0x50) & bit) != 0)
                U16(msg, 7) = (uint16_t)(U16(msg, 7) | 0x400);
        }
    }
    U16(msg, 5) = (uint16_t)U32(entry, 0x150);
    U32(msg, 0xe) = (uint32_t)Render_Ftol((double)FL(entry, 0x58));
    U32(msg, 0x12) = (uint32_t)Render_Ftol((double)FL(entry, 0x5c));
    U32(msg, 0x16) = (uint32_t)Render_Ftol((double)FL(entry, 0x60));
    U32(msg, 0x1a) = (uint32_t)W9b_Fixed(FL(entry, 0x64));
    U32(msg, 0x1e) = (uint32_t)W9b_Fixed(FL(entry, 0x68));
    U32(msg, 0x22) = (uint32_t)W9b_Fixed(FL(entry, 0x6c));
    U32(msg, 0x26) = (uint32_t)Render_Ftol((double)FL(block, 0x520));
    U32(msg, 0x2a) = (uint32_t)Render_Ftol((double)FL(block, 0x588));
    U32(msg, 0x2e) = (uint32_t)W9b_Fixed(FL(block, 0x2c));
    U32(msg, 0x32) = (uint32_t)W9b_Fixed(FL(block, 0x7c));
    U32(msg, 0x36) = (uint32_t)W9b_Fixed(FL(block, 0x38));
    U32(msg, 0x3a) = (uint32_t)W9b_Fixed(FL(block, 8));
    U32(msg, 0x3e) = (uint32_t)W9b_Fixed(FL(block, 0x18));
    U16(msg, 7) = (uint16_t)(U16(msg, 7) | (U32(block, 0x4f0) & 0xf));
    if (U32(block, 0xc1d) != 0) {
        msg[9] |= 4;
        U32(block, 0xc19) = 0;
    }
    if (U32(block, 0xc19) != 0) {
        msg[9] |= U8(block, 0xc19);
        U32(block, 0xc1d) = 0;
    }
    if (U16(block, 0x560) & 0x80)
        U16(msg, 7) = (uint16_t)(U16(msg, 7) | 0x800);
    if ((int8_t)U8(U32(U32(block, 0), 0x54), 0x59) != 0)
        U16(msg, 7) = (uint16_t)(U16(msg, 7) | 0x10);
    if (S32(block, 0x510) > 0 && U32(block, 0x4f0) == 2 && U32(block, 0x514) != 0 && U32(block, 0x518) != 0)
        U16(msg, 7) = (uint16_t)(U16(msg, 7) | 0x100);
    if (U32(entry, 0x15c) != 0)
        msg[9] |= 0x40;
    if (U32(block, 0xc21) != 0) {
        U16(msg, 7) = (uint16_t)(U16(msg, 7) | 0x20);
        U32(block, 0xc25) = 0;
    }
    if (U32(block, 0xc25) != 0) {
        U16(msg, 7) = (uint16_t)(U16(msg, 7) | 0x40);
        U32(block, 0xc21) = 0;
    }
    if (U32(block, 0xc29) == 2) {
        msg[9] |= 8;
        U32(block, 0xc29) = 0;
    } else if (U32(block, 0xc29) == 1) {
        msg[9] |= 0x10;
        U32(block, 0xc29) = 0;
    } else if (U32(block, 0xc2d) != 0) {
        msg[9] |= 0x20;
        U32(block, 0xc2d) = 0;
    }
    U16(msg, 7) = (uint16_t)(U16(msg, 7) | (((uint32_t)U16(block, 0x562) << 12) & 0xf000));
    U32(msg, 0xa) = G32(0x1015dc78);
    Net_DpSend_1007e920(msg, 0x42, guaranteed != 0 ? 1 : 0);
    if (U32(block, 0x4f0) != 4 && U32(block, 0x4f0) != 5)
        Sim_1003ffe9(index);
}
