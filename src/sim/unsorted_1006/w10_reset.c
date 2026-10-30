#include "w10.h"
#include "../simmem.h"
#include "../util/access.h"
#include "../util/util.h"
#include "w8w.h"
#include "../unsorted_1001/w8t.h"
#include "../unsorted_1002/w10.h"
#include "../unsorted_1002/w3o.h"
#include "../unsorted_1002/w6d.h"
#include "../unsorted_1004/w3s.h"
#include "../unsorted_1004/w5c.h"
#include "../unsorted_1009/w5a.h"
#include "../unsorted_100a/w3t.h"
#include "../unsorted_100a/w5x.h"
#include "../unsorted_100a/w5y.h"
#include "../unsorted_100a/w6g.h"
#include "../unsorted_1010/w6k.h"

/* the record's dwords the reset clears (0x500 = 1.0f and 0xc1d / 0xc25 = 1 are set separately, in this order) */
static const uint16_t W10_6CLEAR1[] = {0x4fc, 0x4e8, 0x508, 0x4f4};
static const uint16_t W10_6CLEAR2[] = {0x590, 0x4f0, 0x4d8, 0x4dc, 0x4e0, 0x504, 0x50c, 0x548, 0x54c, 0x550,
                                       0x554, 0x558, 0x55c, 0x544};
static const uint16_t W10_6CLEAR3[] = {0x564, 0x568, 0x56c, 0x570, 0x574, 0x578, 0x57c, 0x580, 0x584, 0x4d0, 0x607,
                                       0xc19};
static const uint16_t W10_6CLEAR4[] = {0xc29, 0xc2d, 0xc31, 0xc15, 0xc3d, 0xc41, 0xc45, 0xc39, 0xc3d, 0xc4d};

// FUNCTION: HGSIM 0x1006da46
void __cdecl Sim_1006da46(uint8_t *obj)
{
    uint8_t *rec = BYTES(U32(obj, 0x28));
    uint32_t unit, node, self, saved = 0;
    double rate;
    unsigned i;

    if (rec == 0)
        return;
    self = G32(0x1017b8c8);
    (void)*(volatile uint32_t *)&U32(PTRTAB(self), 8);     /* read by the original, not used */
    if (G32(0x1014c99c) != U32(obj, 4)) {
        Sim_1009d8b9((const uint32_t *)rec);
        if (U32(U32(rec, 0), 4) == G32(0x1017b8c8))
            Sim_1002bca0();
    }
    unit = U32(rec, 0);
    if (U32(unit, 0) != 1 && U32(unit, 0) != 9)
        U32(rec, 0x53c) = U32(U32(U32(unit, 0x48), 0x8c), 0x6c);
    if (U32(unit, 4) == G32(0x1017b8c8))
        rate = 0.4;
    else if (G32(0x1015ec58) == 0)
        rate = 0.2;
    else
        rate = (U32(unit, 0x14) == 3 && U32(unit, 0xc) == G32(0x1017b8c8)) ? 0.2 : 0.4;
    Sim_10047b76(rec + 4, 0, 0, rate);
    Sim_10047b76(rec + 0x24, 0, 0, 0.2);
    Sim_10047b76(rec + 0x34, 0, 0, 0.3);
    Sim_10047b76(rec + 0x14, 0, 0, 0.2);
    Sim_10047b76(rec + 0x74, 0x3c800000, 0x3c800000, 0.2);
    switch (U32(unit, 0)) {
    case 0xa:
        rate = 0.9;
        break;
    case 8:
        rate = 0.6;
        break;
    case 1:
        rate = 0.7;
        break;
    default:
        rate = 0.3;
        break;
    }
    Sim_10047b76(rec + 0x44, 0, 0, rate);
    Sim_10047b76(rec + 0x54, 0, 0, rate);
    Sim_10047b76(rec + 0x64, 0, 0, 0.6);
    for (i = 0; i < sizeof W10_6CLEAR1 / sizeof W10_6CLEAR1[0]; i++)
        U32(rec, W10_6CLEAR1[i]) = 0;
    U16(rec, 0x560) |= 0x2000;
    for (i = 0; i < sizeof W10_6CLEAR2 / sizeof W10_6CLEAR2[0]; i++)
        U32(rec, W10_6CLEAR2[i]) = 0;
    U32(rec, 0x500) = 0x3f800000;
    for (i = 0; i < sizeof W10_6CLEAR3 / sizeof W10_6CLEAR3[0]; i++)
        U32(rec, W10_6CLEAR3[i]) = 0;
    U32(rec, 0xc1d) = 1;
    U32(rec, 0xc21) = 0;
    U32(rec, 0xc25) = 1;
    for (i = 0; i < sizeof W10_6CLEAR4 / sizeof W10_6CLEAR4[0]; i++)
        U32(rec, W10_6CLEAR4[i]) = 0;
    unit = U32(rec, 0);
    node = U32(unit, 0x48);
    Sim_100a9d7c((void *)(uintptr_t)node, 0.0f, *(float *)(rec + 0x520), 0.0f);
    Sim_100aa799((void *)(uintptr_t)U32(U32(rec, 0), 0x48));
    Sim_100a9ca3((void *)(uintptr_t)U32(U32(rec, 0), 0x48), U32(rec, 0) + 0x64, U32(rec, 0) + 0x68, U32(rec, 0) + 0x6c);
    Sim_100a9cfb((void *)(uintptr_t)U32(U32(rec, 0), 0x48), (uint32_t *)(uintptr_t)(U32(rec, 0) + 0x58),
                 (uint32_t *)(uintptr_t)(U32(rec, 0) + 0x5c), (uint32_t *)(uintptr_t)(U32(rec, 0) + 0x60));
    if (U32(obj, 0) == 0xa) {
        Sim_1002fd4c((void *)(uintptr_t)U32(U32(obj, 0x48), 0x8c), (float *)(obj + 0x284), (float *)(obj + 0x288),
                     (float *)(obj + 0x28c), (float *)(obj + 0x290), (float *)(obj + 0x294), (float *)(obj + 0x298));
        U32(obj, 0x29c) = 1;
    } else if (U32(obj, 0) == 4) {
        if (Sim_1010d240(BYTES(U32(obj, 0x28)) + 0xbeb, SIM_PTR(const uint8_t, 0x101453c8), 8) == 0) {
            Sim_1006d911(obj);
            U32(obj, 0x29c) = 1;
        }
    } else {
        U32(obj, 0x29c) = 0;
    }
    unit = U32(rec, 0);
    U32(unit, 0x78) = 0;
    U32(U32(rec, 0), 0x74) = 0;
    U32(U32(rec, 0), 0x70) = 0;
    unit = U32(rec, 0);
    *(float *)BYTES(U32(rec, 0) + 0x7c) = (float)Sim_1002d961(*(float *)BYTES(unit + 0x58),
                                                              *(float *)BYTES(unit + 0x5c), *(float *)BYTES(unit + 0x60));
    U32(U32(rec, 0), 0x154) = (uint32_t)Sim_100ac21a(1, (void *)(uintptr_t)U32(rec, 0));
    U8(U32(U32(rec, 0), 0x54), 0x34) = 0;
    U8(U32(U32(rec, 0), 0x54), 0x35) = 0;
    U8(U32(U32(rec, 0), 0x54), 0x36) = 0;
    U8(U32(U32(rec, 0), 0x54), 0x3c) = 0;
    U8(U32(U32(rec, 0), 0x54), 0x3f) = 0;
    Sim_1004f0d6((void *)(uintptr_t)U32(rec, 0));
    Sim_100aa454((void *)(uintptr_t)U32(U32(rec, 0), 0x48));
    if (G32(0x1017b8d4) != 0 && U32(U32(rec, 0), 4) == G32(0x1017b8c8)) {
        U32(U32(U32(rec, 0), 0x54), 8) = 0x3c4ccccd;
        U8(U32(U32(rec, 0), 0x54), 0x46) = 1;
        U8(U32(U32(rec, 0), 0x54), 0x31) = 1;
        G32(0x1013aedc) = 1;
    } else {
        U8(U32(U32(rec, 0), 0x54), 0x46) = 0;
        U32(U32(U32(rec, 0), 0x54), 8) = 0;
        U8(U32(U32(rec, 0), 0x54), 0x31) = 0;
    }
    /* a network kind-3 unit is handled as the own player's while Sim_10011093 runs */
    if (G32(0x1015ec58) != 0 && U32(U32(rec, 0), 0x14) == 3) {
        saved = U32(U32(rec, 0), 0xc);
        U32(U32(rec, 0), 0xc) = G32(0x1017b8c8);
    }
    Sim_10011093((void *)(uintptr_t)U32(rec, 0));
    if (G32(0x1015ec58) != 0 && U32(U32(rec, 0), 0x14) == 3)
        U32(U32(rec, 0), 0xc) = saved;
    *(float *)(rec + 0x4d4) = (float)((double)*(float *)(rec + 0x4d4) / *SIM_PTR(float, 0x1013abc4));
}
