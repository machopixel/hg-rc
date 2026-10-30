#include "w9e.h"
#include "w5y.h"
#include "w7n.h"
#include "w8v.h"
#include "../unsorted_100f/w2e.h"
#include "../unsorted_100f/w2j.h"
#include "../unsorted_100f/w6h.h"
#include "../util/access.h"

/* The discrete control slots: 0x6e bytes each from 0x101620c0, count at 0x1015fdf0. */
#define DISCRETE(i) (GBYTES(0x101620c0) + (uint32_t)(i) * 0x6e)
/* The analog control slots: 0x80 bytes each from 0x1015fe00. */
#define ANALOG(i) (GBYTES(0x1015fe00) + (uint32_t)(i) * 0x80)
#define LINE_NO G32(0x1015fdf8)
#define STR(addr) SIM_PTR(const char, addr)

// FUNCTION: HGSIM 0x100543ee
int __cdecl Input_ReadMap(void *stream)
{
    char line[0x100], name[0x28], device[0x28], key[0x14], sign[4];
    char *cut;
    uint8_t *entry, *slot;
    int32_t idx, channel;

    for (;;) {
        if (Input_ReadLine(line, 0xff, stream) == 0)
            return 1;
        if (Sim_100fd710(line, STR(0x10141b14), name) != 1) {
            W9E_SYSERR(0x11, STR(0x10141b1c), LINE_NO, 0);
            return 0;
        }
        if (Str_Compare(name, STR(0x10141b48)) == 0 || Str_Compare(name, STR(0x10141b58)) == 0)
            G32(0x10179ce4) = 1;
        entry = Sim_10053620(name);
        if (entry == 0) {
            W9E_SYSERR(0x11, STR(0x10141b68), LINE_NO, name, 0);
            return 0;
        }
        if (S32(entry, 8) == 0) {
            if (Input_ReadLine(line, 0xff, stream) == 0) {
                W9E_SYSERR(0x11, STR(0x10141b98), LINE_NO, 0);
                return 0;
            }
            if (Sim_100fd710(line, STR(0x10141bc8), sign, device, key) != 3) {
                W9E_SYSERR(0x11, STR(0x10141bd8), LINE_NO, name, 0);
                return 0;
            }
            channel = Sim_1005393a(Input_GetDevice(device), key);
            if (channel != -1) {
                idx = Input_AddAnalog((const uint32_t *)entry, device, channel);
                if (idx < 0)
                    return 0;
                slot = ANALOG(idx);
                S32(slot, 0x10) = sign[0] != '-' ? 1 : -1;
                if (Input_ReadChord(stream, (int32_t *)(slot + 0x18), slot + 0x1c) == 0)
                    return 0;
            } else {
                while (Input_ReadLine(line, 0xff, stream) != 0 && line[0] != '}') {
                }
            }
        } else if (S32(entry, 8) == 1) {
            idx = GS32(0x1015fdf0);
            slot = DISCRETE(idx);
            if (Input_ReadChord(stream, (int32_t *)(slot + 0xe), slot + 0x16) != 0) {
                G32(0x1015fdf0) = G32(0x1015fdf0) + 1;
                if (GS32(0x1015fdf0) > 100) {
                    W9E_SYSERR(0x11, STR(0x10141c08), LINE_NO, 0);
                    return 0;
                }
                U32(slot, 8) = U32(entry, 4);
                U8(slot, 0xc) = 0;
                U32(slot, 0x12) = Str_CompareN((const char *)BYTES(U32(entry, 0)), STR(0x10141c2c), 5) == 0;
                cut = Crt_StrStr(name, STR(0x10141c34));
                if (cut == 0)
                    cut = Crt_StrStr(name, STR(0x10141c3c));
                if (cut != 0) {
                    *cut = 0;
                    entry = Sim_10053620(name);
                    if (entry == 0) {
                        Str_Cat(name, STR(0x10141c44));
                        entry = Sim_10053620(name);
                    }
                    if (entry != 0 && Input_AddAnalog((const uint32_t *)entry, 0, 0) < 0)
                        return 0;
                }
            }
        } else {
            idx = GS32(0x1015fdf0);
            slot = DISCRETE(idx);
            if (Input_ReadChord(stream, (int32_t *)(slot + 0xe), slot + 0x16) != 0) {
                G32(0x1015fdf0) = G32(0x1015fdf0) + 1;
                U32(slot, 8) = U32(entry, 4);
                U8(slot, 0xc) = 1;
                U32(slot, 0x12) = Str_CompareN((const char *)BYTES(U32(entry, 0)), STR(0x10141c4c), 5) == 0;
            }
        }
    }
}
