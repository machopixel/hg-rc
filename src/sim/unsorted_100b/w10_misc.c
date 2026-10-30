#include "w10.h"
#include "../simmem.h"
#include "../util/access.h"
#include "../util/util.h"
#include "../unsorted_1001/w6d.h"
#include "../unsorted_1003/fns.h"
#include "../unsorted_100c/w3t.h"
#include "../unsorted_100c/w5c.h"

#define W10bNOW G32(0x1015dc78)

// FUNCTION: HGSIM 0x100bf28e
void __cdecl Sim_100bf28e(uint8_t *obj)
{
    uint8_t *path;

    U16(obj, 0x1aa) = U16(obj, 0x1a8);
    U32(obj, 0x204) = 0;
    U32(obj, 0x268) = 0;
    U32(obj, 0x218) = 0;
    U32(obj, 0x21c) = 0;
    U32(obj, 0x264) = 0;
    U32(obj, 0x200) = W10bNOW + 0x235a;
    G32(0x1017e2a8) = 0;
    G32(0x1017e2ac) = 0;
    G32(0x1017e2b0) = 0;
    switch ((uint32_t)(int32_t)S16(obj, 0x1f8)) {
    case 0:
        U32(obj, 0x200) = W10bNOW + (uint32_t)(Sim_1009c929(5) + 8) * 0xb5u;
        break;
    case 1:
        U32(obj, 0x200) = W10bNOW + 0xe24;
        U32(obj, 0x21c) = 0xffffffffu;
        break;
    case 3:
        U32(obj, 0x218) = Sim_1009c929(2) != 0 ? 1u : 0xffffffffu;
        break;
    case 4:
        U32(obj, 0x200) = W10bNOW + 0x1c48;
        U32(obj, 0x218) = 1;
        U32(obj, 0x204) = 0;
        break;
    case 5:
        Sim_100c3725(obj);
        U32(obj, 0x264) = 1;
        break;
    case 7:
        if (!(*(float *)(obj + 0xd4) >= *SIM_PTR(float, 0x1012bb14)))
            Sim_100c00c7(obj, U32(obj, 4) | 0x200u, (int16_t)(Sim_1009c929(2) != 0 ? 4 : 12), 0x461c4000u);
        else
            Sim_100c00c7(obj, U32(obj, 4) | 0x200u, (int16_t)(Sim_1009c929(2) != 0 ? 3 : 15), 0x461c4000u);
        break;
    case 8:
        U32(obj, 0x200) = W10bNOW + (uint32_t)(Sim_1009c929(0xb) + 0xa) * 0xb5u;
        U8(U32(obj, 0x54), 0x59) = 1;
        break;
    case 9:
        U32(obj, 0x218) = 0;
        U32(U32(obj, 0x54), 0xc) = 0;
        U32(obj, 0x264) = 1;
        break;
    case 10:
        if (U32(U32(obj, 0x28), 0x4f4) != 0 && S16(obj, 0x1fa) == 0xa && S32(obj, 0x84) == -1) {
            U32(obj, 0x200) = W10bNOW + 0x2d4;
        } else {
            if (U32(obj, 0) != 8)
                U8(U32(obj, 0x54), 0x59) = 1;
            U32(obj, 0x200) = W10bNOW + 0x5a8;
        }
        break;
    case 11:
        U32(U32(obj, 0x54), 0xc) = 0;
        U32(obj, 0x264) = 1;
        if (U32(obj, 0x218) == 0 || U32(obj, 0x218) == 2)
            U8(U32(obj, 0x54), 0x1f) = 1;
        else
            U8(U32(obj, 0x54), 0x21) = 1;
        Sim_100c274a(obj, 1);
        U32(obj, 0x200) = W10bNOW + 0x16a;
        break;
    case 12:
        U32(obj, 0x200) = W10bNOW + 0x46b4;
        Sim_100157ca(obj);
        break;
    case 13:
        U32(obj, 0x200) = W10bNOW + 0x46b4;
        U32(obj, 0x218) = (uint32_t)Sim_1009c929(0x10);
        break;
    case 14:
        U32(obj, 0x200) = 0;
        U32(obj, 0x218) = 1;
        U32(obj, 0x21c) = 0xffffffffu;
        U32(obj, 0x220) = 0x80000000u;
        U16(obj, 0x22c) = 0;
        U32(obj, 0x22e) = 0xffffffffu;
        U32(obj, 0x23a) = 0xffffffffu;
        break;
    case 16:
        if (S32(obj, 0x23a) >= 0 && (path = Sim_10033c42(S32(obj, 0x23a))) != 0)
            Sim_1003359f(path, obj);
        U32(obj, 0x200) = W10bNOW + 0x712;
        break;
    case 17:
        U32(obj, 0x200) = W10bNOW + 0x3fa2;
        U32(obj, 0x204) = W10bNOW + 0x5a8;
        U32(obj, 0x218) = 0;
        U32(obj, 0x218) = 0x11;
        U32(obj, 0x21c) = 0;
        U32(obj, 0x224) = 0;
        U32(obj, 0x228) = 0x80000000u;
        if (S32(obj, 0x23a) >= 0 && (path = Sim_10033c42(S32(obj, 0x23a))) != 0)
            Sim_1003359f(path, obj);
        break;
    case 19:
        U32(obj, 0x200) = W10bNOW + 0x2a6c;
        U32(obj, 0x218) = 1;
        U32(obj, 0x21c) = 0;
        break;
    case 20:
        U32(obj, 0x200) = W10bNOW + 0xe24;
        U32(obj, 0x21c) = 0;
        U32(obj, 0x218) = 1;
        U32(obj, 0x204) = 0;
        break;
    case 21:
        U32(obj, 0x200) = W10bNOW + 0x389;
        U32(obj, 0x218) = 1;
        U32(obj, 0x204) = 0;
        break;
    case 22:
    case 23:
        U32(obj, 0x200) = 0;
        U32(obj, 0x218) = 1;
        U32(obj, 0x21c) = 0x2710;
        U32(obj, 0x220) = 0xffffffffu;
        U32(obj, 0x224) = 0x80000000u;
        break;
    case 24:
    case 25:
        U32(obj, 0x200) = W10bNOW + 0x2d4;
        break;
    case 26:
        U32(obj, 0x200) = W10bNOW + 0x43e;
        U32(obj, 0x218) = 0;
        break;
    case 27:
        U32(obj, 0x204) = W10bNOW + (uint32_t)(Sim_1009c929(4) + 1) * 0x5au;
        U32(obj, 0x200) = U32(obj, 0x204);
        U32(obj, 0x218) = (uint32_t)Sim_1009c929(2);
        break;
    default:     /* 2, 6, 15, 18 and anything else */
        break;
    }
}
