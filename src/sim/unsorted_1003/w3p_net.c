#include "w3p.h"
#include "fns.h"
#include "../render/w2n.h"
#include "../unsorted_1004/fns.h"
#include "../unsorted_1007/w3p.h"
#include "../unsorted_100f/w2e.h"
#include "../util/access.h"
#include "../util/w3p.h"

#define ROOT G32(0x1015d004)
#define SELF G32(0x1017b8c8)

// FUNCTION: HGSIM 0x1003fcdc
int __cdecl Sim_1003fcdc(int target, const char *text)
{
    uint8_t *buf;
    uint32_t len = Str_Length(text);
    int i;

    if (len >= 0x50)
        return 0;
    len += 3;
    buf = BYTES(G32(0x1015ecc8));
    Mem_Move(buf, GBYTES(0x1013ad4c), 2);
    Str_Copy((char *)buf + 2, text);
    switch ((uint32_t)target + 4u) {
    case 0:
        break;
    case 1:
        for (i = 0; i < S32(ROOT, 0x12); i++) {
            uint32_t id = U16(BYTES(U32(ROOT, 0x126)), (uint32_t)i * 0x1e + 0x1a);

            if (PTRTAB(id) != 0 && U32(PTRTAB(id), 0x14) == 1 && Sim_10052be7((int)id) == 1)
                Net_DpSend_1007e950(U16(BYTES(U32(ROOT, 0x16)), (uint32_t)i * 2), buf, (int)len, 1);
        }
        break;
    case 2:
        for (i = 0; i < S32(ROOT, 0x12); i++) {
            uint32_t id = U16(BYTES(U32(ROOT, 0x126)), (uint32_t)i * 0x1e + 0x1a);

            if (PTRTAB(id) != 0 && U32(PTRTAB(id), 0x14) == 1 && Sim_10052be7((int)id) == 0 && id != SELF)
                Net_DpSend_1007e950(U16(BYTES(U32(ROOT, 0x16)), (uint32_t)i * 2), buf, (int)len, 1);
        }
        break;
    case 3:
        Net_DpSend_1007e920(buf, (int)len, 1);
        break;
    default:
        if (U32(PTRTAB(target), 0x14) == 1)
            Net_DpSend_1007e950(U16(BYTES(U32(ROOT, 0x16)), (uint32_t)Sim_1003dbc0((uint32_t)target) * 2), buf, (int)len, 1);
        break;
    }
    return 1;
}

// FUNCTION: HGSIM 0x1003ffe9
void __cdecl Sim_1003ffe9(int index)
{
    uint8_t *msg = BYTES(G32(0x1015ecb0));
    uint8_t *entry;
    uint8_t *block;
    int changed = 0;
    int i;

    Mem_Move(msg, GBYTES(0x1013ad64), 2);
    entry = BYTES(PTRTAB(index));
    if (entry == 0)
        return;
    block = BYTES(U32(entry, 0x28));
    msg[2] = (uint8_t)index;
    U16(msg, 3) = (uint16_t)U32(entry, 0x150);
    for (i = 0; i < 0x11; i++) {
        uint8_t *e = BYTES(U32(block, 0x88) + (uint32_t)i * 0x3c);
        int32_t divisor = S32(e, 0x38);
        int32_t level;

        if (divisor != 0) {
            double sum = (double)F32(e, 8) * (double)GF32(0x1011150c) + (double)F32(e, 0) * (double)GF32(0x1011150c);

            level = (int32_t)(0xfu - (uint32_t)Render_Ftol(sum * (double)GF32(0x10111530) / (double)divisor));
        } else {
            level = 0;
        }
        if (level < 0)
            level = 0;
        if (level > 0xf)
            level = 0xf;
        if (X87_LessOrUnordered((double)F32(e, 8), (double)GF32(0x10111534)))
            msg[5 + i] = (uint8_t)(level | 0x10);
        else
            msg[5 + i] = (uint8_t)level;
        if (msg[5 + i] != GBYTES(0x1015da45)[U32(entry, 4) * 0x16u + (uint32_t)i]) {
            changed = 1;
            GBYTES(0x1015da45)[U32(entry, 4) * 0x16u + (uint32_t)i] = msg[5 + i];
        }
    }
    if (changed != 0)
        Net_DpSend_1007e920(msg, 0x16, 1);
}
