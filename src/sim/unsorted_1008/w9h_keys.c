#include "w9h.h"
#include "../util/access.h"
#include "../unsorted_1002/w8u.h"
#include "../unsorted_1003/fns.h"
#include "../unsorted_1004/fns.h"
#include "../unsorted_1004/w6e.h"
#include "../unsorted_1007/w2e.h"
#include "../unsorted_100f/w8z.h"
#ifdef _MSC_VER
#include <intrin.h>
#endif

#define W9H_TOGGLE(addr) (G32(addr) = (G32(addr) == 0))

/* Formats `text` into the 40-byte buffer of the caller and shows it for 0x16a ticks. */
static void W9h_Announce(char *buf, const char *text)
{
    Crt_SPrintf(buf, text);
    Sim_1007df6c(buf, (const char *)BYTES(G32(0x109b98c0)), 0x16a, 0x32);
}

/* The byte of the mission table the two debug keys 0x98 / 0x99 change. */
static uint8_t *W9h_MissionByte(void)
{
    return GBYTES(0x1047383a) + G32(0x101303d0) * 0x4fc3au + G32(0x1015d108) * 0x13fu;
}

// FUNCTION: HGSIM 0x1008b922
void __cdecl Sim_1008b922(uint32_t key)
{
    char buf[40];

    switch (key & 0xffff) {
    case 0x15:
        Sim_100448cd(2);
        break;
    case 0x17:
        Sim_10044a69(1, 0);
        break;
    case 0x18:
        Sim_10044a69(0, 0);
        break;
    case 0x26:
#ifdef _MSC_VER
        __debugbreak();
#else
        __asm__ volatile("int3");
#endif
        break;
    case 0x79:
    case 0x7a:
    case 0x7b:
    case 0x7c:
    case 0x7d:
    case 0x7e:
    case 0x7f:
    case 0x80:
    case 0x81:
        G32(0x1014b930) = (key & 0xffff) - 0x78;
        break;
    case 0x82:
        G32(0x1014b930) = 0;
        break;
    case 0x83:
        W9h_Announce(buf, (const char *)BYTES(G32(0x101c4fa4)));
        G32(0x1014b934) = 1;
        break;
    case 0x84:
        W9h_Announce(buf, (const char *)BYTES(G32(0x101c4fa8)));
        G32(0x1014b934) = 2;
        break;
    case 0x85:
        W9h_Announce(buf, (const char *)BYTES(G32(0x101c4f98)));
        G32(0x1014b934) = 3;
        break;
    case 0x86:
        W9h_Announce(buf, (const char *)BYTES(G32(0x101c4fa0)));
        G32(0x1014b934) = 4;
        break;
    case 0x87:
        W9h_Announce(buf, (const char *)BYTES(G32(0x101c4fb8)));
        G32(0x1014b934) = 5;
        break;
    case 0x88:
        W9h_Announce(buf, (const char *)BYTES(G32(0x101c4fbc)));
        G32(0x1014b934) = 6;
        break;
    case 0x89:
        W9h_Announce(buf, (const char *)BYTES(G32(0x101c4fac)));
        G32(0x1014b934) = 7;
        break;
    case 0x8a:
        W9h_Announce(buf, (const char *)BYTES(G32(0x101c4fb0)));
        G32(0x1014b934) = 8;
        break;
    case 0x8b:
    case 0x8c:
    case 0x8d:
    case 0x8e:
    case 0x8f:
    case 0x90:
    case 0x91:
    case 0x92:
    case 0x93:
    case 0x94:
        G32(0x101303d0) = (key & 0xff) - 0x8b;
        if (GS32(0x101303d0) >= GS32(0x104737e0))
            G32(0x101303d0) = 0xffffffffu;
        break;
    case 0x95:
        G32(0x101303d0) = (uint32_t)(G32(0x101303d0) != 0xffffffffu) - 2u;
        break;
    case 0x96:
        G32(0x1015d108) -= 1;
        break;
    case 0x97:
        G32(0x1015d108) += 1;
        break;
    case 0x98:
        if (G32(0x101303d0) != 0xffffffffu && *W9h_MissionByte() == 3)
            *W9h_MissionByte() = 6;
        break;
    case 0x99:
        if (G32(0x101303d0) != 0xffffffffu && *W9h_MissionByte() == 3)
            *W9h_MissionByte() = 5;
        break;
    case 0x9a:
        Sim_1003327f(0x100);
        break;
    case 0x9b:
        Sim_1003327f(0x200);
        break;
    case 0x9c:
        Sim_1003327f(0x400);
        break;
    case 0x9d:
        Sim_1003327f(0x800);
        break;
    case 0x9f:
        W9H_TOGGLE(0x1012dc98);
        G32(0x1015d04c) = G32(0x1012dc98);
        break;
    case 0xa1:
        if (G32(0x1014cb20) == 0)
            G32(0x1014cb20) = 1;
        else if (G32(0x1014cb20) == 1)
            G32(0x1014cb20) = 2;
        else
            G32(0x1014cb20) = 0;
        break;
    case 0xa2:
        W9H_TOGGLE(0x1015d02c);
        G32(0x1015d030) = G32(0x1015d02c);
        break;
    case 0xa3:
        U16(G32(0x109e88e0), 6) = (uint16_t)(S16(G32(0x109e88e0), 6) == 0);
        break;
    case 0xa4:
        W9H_TOGGLE(0x1015d034);
        G32(0x1015d038) = G32(0x1015d034);
        break;
    case 0xa5:
        W9H_TOGGLE(0x1015d040);
        G32(0x1015d044) = G32(0x1015d040);
        break;
    case 0xa8:
        if (U8(G32(0x1017b8c4), 2) == 0) {
            U8(G32(0x1017b8c4), 2) = 1;
            W9h_Announce(buf, (const char *)BYTES(G32(0x101c4fe0)));
        } else {
            U8(G32(0x1017b8c4), 2) = 0;
            W9h_Announce(buf, (const char *)BYTES(G32(0x101c4824)));
        }
        break;
    case 0xa9:
        if (U8(G32(0x1017b8c4), 3) == 0) {
            U8(G32(0x1017b8c4), 3) = 1;
            W9h_Announce(buf, (const char *)BYTES(G32(0x101c482c)));
        } else {
            U8(G32(0x1017b8c4), 3) = 0;
            W9h_Announce(buf, (const char *)BYTES(G32(0x101c4828)));
        }
        break;
    case 0xaa:
        if (U8(G32(0x1017b8c4), 4) == 0) {
            U8(G32(0x1017b8c4), 4) = 1;
            W9h_Announce(buf, (const char *)BYTES(G32(0x101c4834)));
        } else {
            U8(G32(0x1017b8c4), 4) = 0;
            W9h_Announce(buf, (const char *)BYTES(G32(0x101c4830)));
        }
        break;
    case 0xab:
        if (G32(0x10152418) == 2) {
            G32(0x10152418) = 1;
            W9h_Announce(buf, SIM_PTR(const char, 0x1014b980));
        } else {
            G32(0x10152418) = 2;
            W9h_Announce(buf, (const char *)BYTES(G32(0x101c4838)));
        }
        break;
    case 0xac:
        Cache_DumpLog();
        break;
    case 0xae:
        W9H_TOGGLE(0x10152414);
        break;
    case 0xaf:
        W9H_TOGGLE(0x1015d02c);
        break;
    case 0xb1:
        W9H_TOGGLE(0x1015d040);
        break;
    case 0xb2:
        W9H_TOGGLE(0x1015d034);
        break;
    case 0xb3:
        if (G32(0x1014cb1c) == 0)
            G32(0x1014cb1c) = 2;
        else if (G32(0x1014cb1c) == 1)
            G32(0x1014cb1c) = 0;
        else
            G32(0x1014cb1c) = 1;
        break;
    case 0xb4:
        W9H_TOGGLE(0x1015d01c);
        break;
    default:
        break;
    }
}
