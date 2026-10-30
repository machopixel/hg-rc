#include "w9b.h"
#include "w2i.h"
#include "w3p.h"
#include "../unsorted_1001/w8t.h"
#include "../unsorted_1003/w9b.h"
#include "../unsorted_1007/w2e.h"
#include "../unsorted_1007/w3p.h"
#include "../unsorted_100a/w5b.h"
#include "../unsorted_100a/w5x.h"
#include "../unsorted_100a/w5y.h"
#include "../unsorted_100f/w2e.h"
#include "../unsorted_100f/w8z.h"
#include "../util/access.h"

#define ROOT G32(0x1015d004)
#define SELF G32(0x1017b8c8)
#define LIST_COUNT S32(ROOT, 0x12)
#define LIST_ID(i) U16(BYTES(U32(ROOT, 0x126)), (uint32_t)(i) * 0x1e + 0x1a)
#define TEXT(addr) SIM_PTR(const char, addr)
#define TEXT_AT(addr) ((const char *)(uintptr_t)G32(addr))

// FUNCTION: HGSIM 0x1004227f
void __cdecl Net_UpdatePlayers(void)
{
    int due = 0;
    int i;
    uint8_t *entry;
    uint8_t *block;
    char text[0x50];

    if ((int32_t)(G32(0x1015dc7c) - G32(0x1015ecec)) >= 0) {
        due = 1;
        G32(0x1015ecec) = G32(0x1015dc7c) + G32(0x1013ac9c);
    }
    for (i = 0; i < GS32(0x1015dc64); i++) {
        entry = BYTES(PTRTAB(i));
        block = BYTES(U32(entry, 0x28));
        if (U32(entry, 0x164) != 0 && (S16(entry, 0x1c) & 2) == 0 && (S16(entry, 0x1c) & 4) == 0)
            U32(entry, 0x164) = 0;
        if (U32(entry, 0x14) == 3 && U32(entry, 0xc) == SELF && due != 0) {
            Net_SendStateMessage(i, 0);
            Sim_10040376(i);
            Sim_100408bd(i);
            if (U32(entry, 0x164) == 0 && ((S16(entry, 0x1c) & 2) != 0 || (S16(entry, 0x1c) & 4) != 0)) {
                int killer = (int16_t)U32(entry, 0x150);

                if (killer >= 0 && killer < 8 && killer < GS32(0x1015dc64)) {
                    if (killer == S32(entry, 4))
                        Crt_SPrintf(text, TEXT(0x1013ae28), (const char *)entry + 0x124, TEXT_AT(0x101c4a04));
                    else
                        Crt_SPrintf(text, TEXT(0x1013ae30), (const char *)entry + 0x124, TEXT_AT(0x101c49f0),
                                    (const char *)BYTES(PTRTAB(killer)) + 0x124);
                } else {
                    Crt_SPrintf(text, TEXT(0x1013ae40), (const char *)entry + 0x124, TEXT_AT(0x101c4a04));
                }
                Sim_1007df6c(text, TEXT_AT(0x109b98c0), 0x712, 0x50);
                GBYTES(0x1015d1b0)[U32(entry, 4)] = 1;
                if (U8(G32(0x1017b8c4), 8) != 0 && U8(U32(entry, 0x168), 0) != 0) {
                    Net_SendStateMessage((int)U32(entry, 4), 1);
                    if (G32(0x1015ecd0) == 2) {
                        Sim_10040ab8((int)U32(entry, 4));
                        U32(entry, 0x164) = 1;
                    } else {
                        W9B4_CALL_1009d200((int)U32(entry, 4), 0, 0, 0);
                        Sim_10011093(entry);
                        Sim_100aa39f(BYTES(U32(entry, 0x48)));
                        Sim_100aa454(BYTES(U32(entry, 0x48)));
                        Sim_100aa799(BYTES(U32(entry, 0x48)));
                        U16(entry, 0x1c) = (uint16_t)(U16(entry, 0x1c) & 0xb7ff);
                        U32(entry, 0x164) = 1;
                        Crt_SPrintf(text, TEXT(0x1013ae48), (const char *)entry + 0x124);
                        Sim_1007df6c(text, TEXT_AT(0x109b98c0), 0x712, 0x50);
                        GBYTES(0x1015d1b0)[U32(entry, 4)] = 0;
                        Sim_10040564((int)U32(entry, 4));
                        U8(U32(entry, 0x168), 0) = (uint8_t)(U8(U32(entry, 0x168), 0) - 1);
                    }
                }
            }
        }
        if (G32(0x1015ecd0) == 1 && U32(entry, 0x14) == 3 && (S16(entry, 0x1c) & 0x4000) == 0 &&
            (U32(block, 0x4f0) & 2) != 0 && (S32(entry, 0xc) < 0 || U32(PTRTAB(S32(entry, 0xc)), 0x160) != 0) &&
            U32(entry, 0x18) == 0) {
            uint8_t *buf;

            /* the next player of the list that is free (no +0x160 mark, state 0 or 1), round robin */
            for (;;) {
                uint8_t *e = BYTES(PTRTAB(LIST_ID(G32(0x1015ec78))));

                if (U32(e, 0x160) == 0 && (U32(e, 0x14) == 0 || U32(e, 0x14) == 1))
                    break;
                G32(0x1015ec78) += 1;
                if ((int32_t)G32(0x1015ec78) >= LIST_COUNT)
                    G32(0x1015ec78) = 0;
            }
            buf = BYTES(G32(0x1015eca0));
            buf[2] = (uint8_t)i;
            buf[3] = (uint8_t)U32(entry, 0xc);
            buf[4] = (uint8_t)LIST_ID(G32(0x1015ec78));
            Sim_10040645(buf + 5, entry);
            if (LIST_ID(G32(0x1015ec78)) == SELF) {
                Mem_Move(buf, GBYTES(0x1013ae5c), 2);
                Sim_10040564((int)U32(entry, 4));
            } else {
                Mem_Move(buf, GBYTES(0x1013ae60), 2);
                U32(entry, 0x18) = 1;
            }
            Net_DpSend_1007e920(buf, 0x23, 1);
            G32(0x1015ec78) += 1;
            if ((int32_t)G32(0x1015ec78) >= LIST_COUNT)
                G32(0x1015ec78) = 0;
        }
        if (U32(entry, 0x14) == 3 && U32(entry, 0xc) == SELF && (U32(block, 0x4f0) & 2) != 0 && U32(entry, 0x18) == 0) {
            uint32_t kind = U32(entry, 0xf0) & 0xf00;
            uint32_t target = U32(entry, 0xf0) & 0xff;

            if (kind == 0x200 && target != SELF) {
                uint8_t *other = BYTES(PTRTAB(target));

                if (U32(other, 0x14) == 1) {
                    uint8_t *buf = BYTES(G32(0x1015eca0));

                    U32(entry, 0x18) = 1;
                    Mem_Move(buf, GBYTES(0x1013ae64), 2);
                    buf[2] = (uint8_t)i;
                    buf[3] = (uint8_t)SELF;
                    buf[4] = (uint8_t)target;
                    Sim_10040645(buf + 5, entry);
                    Net_DpSend_1007e920(buf, 0x23, 1);
                }
            }
        }
    }
}
