#include "w3r.h"
#include "../util/w3r.h"
#include "../render/w2n.h"

// FUNCTION: HGSIM 0x10054d98
int __cdecl Sim_10054d98(void *param)
{
    uint8_t *p = (uint8_t *)(uintptr_t)U32(param, 0x14);
    int state = (int8_t)U8(p, 0x15);
    uint32_t flag;

    flag = U32(p, 0x2a);
    if (flag != 0 && (int8_t)U8(flag, 0) != 0) {
        if (U32(param, 8) != 0) {
            uint32_t slot = U32(param, 0);
            uint32_t arg = U32(param, 4);

            ((void(__cdecl *)(uint32_t, uint32_t))(uintptr_t)U32(G32(0x1015fd20u + slot * 4u), 0x10))(G32(0x1015f8f0u + slot * 0x74u), arg);
        }
        S32(U32(p, 4), 0) = 0;
        U32(p, 0x1a) = U32(p, 0x10);
        state = 1;
    } else {
        flag = U32(p, 0x2e);
        if (flag != 0 && (int8_t)U8(flag, 0) != 0) {
            int32_t n = Render_Ftol((double)F32(U32(p, 0), 0) * (double)GF32(0x1012b458) + (double)GF32(0x1012b45c));
            double d;

            n = (int32_t)((uint32_t)n << ((int8_t)U8(p, 0x14) & 31));
            d = (double)n;
            d = d - (double)S32(p, 0xc);
            d = d * GF64(0x1012b460);
            d = d / (double)S32(p, 8);
            d = d - GF64(0x1012b468);
            S32(U32(p, 4), 0) = 0;
            S32(p, 0x1a) = Render_Ftol(d);
            state = 1;
        }
    }
    if ((int8_t)U8(p, 0x15) == 0) {
        flag = U32(p, 0x22);
        if (flag != 0 && (int8_t)U8(flag, 0) != 0) {
            if (S32(U32(p, 4), 0) < 0)
                S32(U32(p, 4), 0) = 0;
            S32(U32(p, 4), 0) = (int32_t)((uint32_t)S32(U32(p, 4), 0) + ((G32(0x1015dc80) * 3u) << (U32(p, 0x1e) & 31)));
            if (S32(U32(p, 4), 0) > 0x8000)
                S32(U32(p, 4), 0) = 0x8000;
            S32(p, 0x1a) = (int32_t)((uint32_t)S32(p, 0x1a) + (uint32_t)S32(U32(p, 4), 0));
            if (S32(p, 0x1a) > 0x10000)
                S32(p, 0x1a) = 0x10000;
            state = 1;
        }
        flag = U32(p, 0x26);
        if (flag != 0 && (int8_t)U8(flag, 0) != 0) {
            if (S32(U32(p, 4), 0) > 0)
                S32(U32(p, 4), 0) = 0;
            S32(U32(p, 4), 0) = (int32_t)((uint32_t)S32(U32(p, 4), 0) - ((G32(0x1015dc80) * 3u) << (U32(p, 0x1e) & 31)));
            if (S32(U32(p, 4), 0) < -0x8000)
                S32(U32(p, 4), 0) = -0x8000;
            S32(p, 0x1a) = (int32_t)((uint32_t)S32(p, 0x1a) + (uint32_t)S32(U32(p, 4), 0));
            if (S32(p, 0x1a) < -0x10000)
                S32(p, 0x1a) = (int32_t)0xffff0000u;
            state = 1;
        }
        if (state == 0)
            S32(U32(p, 4), 0) = 0;
    }
    U8(p, 0x15) = (uint8_t)state;
    return state;
}

static void ScaleRecords(uint32_t gear, int factor)
{
    uint32_t rec = U32(gear, 0x88);
    int i;

    for (i = 0; i < 0x11; i++) {
        uint32_t v;

        F32(rec, 8) = (float)((double)factor * (double)F32(rec, 8));
        v = (uint32_t)Render_Ftol(((double)F32(rec, 8) * (double)GF32(0x1012b4c0) + (double)F32(rec, 0) * (double)GF32(0x1012b4c0)) / (double)GF32(0x1012b4c4));
        if (v > 0xffffu)
            v = 0xffffu;
        U32(rec, 0x38) = v;
        rec += 0x3c;
    }
}

// FUNCTION: HGSIM 0x1005b98c
void __cdecl Sim_1005b98c(int index)
{
    uint32_t gear = U32(PTRTAB(index), 0x28);
    uint32_t player = U32(PTRTAB(GS32(0x1017b8c8)), 8);

    if (G32(0x1015ec58) == 0 && U32(PTRTAB(index), 8) == player) {
        int factor; /* unset for other values of the byte, as in the original */

        switch (U8(G32(0x1017b8c4), 5)) {
        case 0:
            factor = 12;
            break;
        case 1:
            factor = 6;
            break;
        case 2:
            factor = 1;
            break;
        }
        ScaleRecords(gear, factor);
    } else if (G32(0x1015ec58) != 0) {
#ifdef NETA_MUTATE /* the checker's negative check only: 1.0's x4 for the own gear and the kind-1 gears */
        if (index == GS32(0x1017b8c8) || S32(U32(gear, 0), 0x14) == 1)
            ScaleRecords(gear, 4);
#else
        /* 1.2 (0x1004e4cc): in a network game every gear's parts get x2 */
        ScaleRecords(gear, 2);
#endif
    }
}
