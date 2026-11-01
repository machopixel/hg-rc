#include "w14.h"
#include "fns.h"
#include "w6e.h"
#include "../simmem.h"
#include "../util/access.h"
#include "../util/util.h"
#include "../view/view.h"
#include "../render/w33.h"
#include "../unsorted_1000/w5a.h"
#include "../unsorted_1001/w2k.h"
#include "../unsorted_1001/w3r.h"
#include "../unsorted_1001/w5b.h"
#include "../unsorted_1001/w5x.h"
#include "../unsorted_1001/w5y.h"
#include "../unsorted_1001/w5z.h"
#include "../unsorted_1001/w6d.h"
#include "../unsorted_1002/w2i.h"
#include "../unsorted_1002/w2j.h"
#include "../unsorted_1002/w5a.h"
#include "../unsorted_1003/w6e.h"
#include "../unsorted_1006/w8w.h"
#include "../unsorted_1006/w9b.h"
#include "../unsorted_1007/fns.h"
#include "../unsorted_1007/w2e.h"
#include "../unsorted_1007/w3r.h"
#include "../unsorted_1007/w5z.h"
#include "../unsorted_1007/w8w.h"
#include "../unsorted_1008/w9h.h"
#include "../unsorted_1009/w2f.h"
#include "../unsorted_1009/w2g.h"
#include "../unsorted_1009/w2j.h"
#include "../unsorted_1009/w3t.h"
#include "../unsorted_100b/w8y.h"
#include "../unsorted_100c/w2h.h"
#include "../unsorted_100d/fns.h"

/* sprintf of the DLL (left original): the messages are used as formats, as the original does. */
#define W14K_SPRINTF SIM_FN(int(__cdecl *)(char *, const char *, ...), 0x100fc780)
typedef void(__cdecl *W14kSoundFn)(uint32_t flags, uint32_t on);

#define W14K_PLAYER() PTRTAB(G32(0x1017b8c8))
#define W14K_REPORTS() ((U16(W14K_PLAYER(), 0x1c) & 0x2000) != 0)
#define W14K_TEXT(addr) ((const char *)(uintptr_t)G32(addr))

/* A message in the label font: sprintf'd into a stack buffer first, as the original does. */
static void W14K_Message(uint32_t text, int ticks)
{
    /* The original's buffer is 40 bytes; the network message (43 characters) runs 4 bytes past it into a local
     * that path never reads (DECISIONS.md, round 14), so an ordinary buffer gives the same result. */
    char buf[64];

    W14K_SPRINTF(buf, W14K_TEXT(text));
    Sim_1007df6c(buf, W14K_TEXT(0x109b98c0), ticks, 0x32);
}

static void W14K_Sound(uint32_t name, int which)
{
    if (W14K_REPORTS())
        Sim_1000ebfb(SIM_PTR(const char, name), 0, 0, which, 0x32);
}

static void W14K_View(uint32_t obj, uint32_t name)
{
    Sim_10077fd8((void *)(uintptr_t)G32(obj));
    W14K_Sound(name, 0);
}

static void W14K_PaneMode(uint32_t mode, uint32_t name, int which)
{
    uint32_t pane = G32(0x109e88e8);

    U32(pane, 0x30) = U32(pane, 0x30) != mode ? mode : 0;
    W14K_Sound(name, which);
}

/* The camera commands work in single player only. */
#define W14K_SOLO(call)                                                                                                \
    do {                                                                                                               \
        if (G32(0x1015ec58) == 0)                                                                                      \
            call;                                                                                                      \
    } while (0)

/* The lance orders 0x5d..0x63 after the target group is picked: none (-1), already busy (bit 1 of +0x1c), or
 * order it: Sim_10074c3d(order) in single player, in a network game the group itself for a type-1 unit or, when the
 * player leads (Sim_10052aaf([0x1013bb40])), Sim_10074c3d(net). */
static void W14K_Order(uint32_t group, uint32_t none, uint32_t busy, uint32_t ok, int order, int net)
{
    if (group == 0xffffffffu) {
        Sim_1000ebfb(SIM_PTR(const char, none), 0, 0, 0, 0x32);
    } else if ((U16(PTRTAB(group), 0x1c) & 2) == 0) {
        Sim_1000ebfb(SIM_PTR(const char, ok), 0, 0, 0, 0x32);
        if (G32(0x1015ec58) == 0)
            G32(0x1017e5ec) = (uint32_t)Sim_10074c3d(order);
        else if (U32(PTRTAB(group), 0x14) == 1)
            G32(0x10151d50) = group;
        else if ((uint32_t)Sim_10052aaf((int)G32(0x1013bb40)) == G32(0x1017b8c8))
            G32(0x1017e5ec) = (uint32_t)Sim_10074c3d(net);
    } else {
        Sim_1000ebfb(SIM_PTR(const char, busy), 0, 0, 0, 0x32);
    }
}

/* Orders 0x5e..0x63: group n in single player; in a network game group n (net order a) while the player has fewer
 * than n groups, else group n - 1 (net order b). */
static void W14K_LanceOrder(uint32_t unit, int n, int order, int a, int b, uint32_t none, uint32_t busy, uint32_t ok)
{
    uint32_t group;
    int net = 0;

    if (U32(unit, 0xc3d) != 0)
        return;
    if (G32(0x1015ec58) == 0) {
        group = (uint32_t)Sim_10015c61(n);
    } else if (Sim_10015cd2(G32(0x1017b8c8)) < n) {
        group = (uint32_t)Sim_10015c61(n);
        net = a;
    } else {
        group = (uint32_t)Sim_10015c61(n - 1);
        net = b;
    }
    W14K_Order(group, none, busy, ok, order, net);
}

/* Cycles a 0 / 1 / 2 state (2 only in a network game) while the map [0x10144fa0] is shown. */
static void W14K_Cycle(uint32_t addr)
{
    G32(0x10151d50) = 0xfffffffeu;
    if (G32(0x10144fa0) == 0)
        return;
    if (G32(addr) == 0)
        G32(addr) = 1;
    else if (G32(addr) == 1)
        G32(addr) = G32(0x1015ec58) == 0 ? 0 : 2;
    else if (G32(addr) == 2)
        G32(addr) = 0;
}

// FUNCTION: HGSIM 0x1004954e
void __cdecl Sim_1004954e(uint32_t cmd)
{
    int32_t throttle = -1;
    uint32_t unit = U32(W14K_PLAYER(), 0x28);
    uint32_t group;
    int net = 0;

    if (G32(0x1015ec58) != 0 && (G32(0x1017b94c) & 1) == 0 && cmd != 0x76 && cmd != 0x4b && cmd != 0x4c) {
        W14K_Message(0x101c45c0, 0x16a);
        return;
    }
    switch (cmd) {
    case 0:
        break;
    case 1:
        *GBYTES(0x1015fd50) = 1;
        *GBYTES(0x1015fd54) = 1;
        *GBYTES(0x1015fdba) = 1;
        *GBYTES(0x1015fdbd) = 1;
        *GBYTES(0x1015fdc4) = 1;
        *GBYTES(0x1015fdc7) = 1;
        break;
    case 2:
        W14K_View(0x109e88e8, 0x1013b718);
        break;
    case 3:
        W14K_View(0x109e890c, 0x1013b70c);
        break;
    case 4:
        Sim_10077fd8((void *)(uintptr_t)G32(0x109e894c));
        if (W14K_REPORTS())
            break;
        /* without the report bit the original goes on into command 5 */
        /* fall through */
    case 5:
        W14K_View(0x109e8950, 0x1013b6dc);
        break;
    case 6:
        W14K_View(0x109e8954, 0x1013b6e8);
        break;
    case 7:
        W14K_View(0x109e8938, 0x1013b700);
        break;
    case 8:
        W14K_View(0x109e8928, 0x1013b6f4);
        break;
    case 9:
        Sim_10077fff();
        break;
    case 10:
        Sim_10077d50();
        G32(0x10144fa0) = 2;
        Sim_1007df6c(W14K_TEXT(0x101c45bc), 0, 0x21f, 0x32);
        break;
    case 0xb:
        G32(0x10144fac) = G32(0x10144fac) == 0;
        W14K_Sound(0x1013b730, 0);
        break;
    case 0xc:
        Sim_1002c10e();
        W14K_Sound(0x1013b724, 0);
        break;
    case 0xd:
        W14K_PaneMode(2, 0x1013b73c, 1);
        break;
    case 0xe:
        W14K_PaneMode(3, 0x1013b748, 2);
        break;
    case 0xf:
        W14K_PaneMode(5, 0x1013b754, 3);
        break;
    case 0x10:
        W14K_PaneMode(7, 0x1013b760, 4);
        break;
    case 0x11:
        U32(G32(0x109e88e8), 0x30) = U32(G32(0x109e88e8), 0x30) != 1;
        W14K_Sound(0x1013b76c, 6);
        break;
    case 0x12:
        G32(0x10151cf8) ^= 1;
        W14K_Sound(0x1013b778, 7);
        break;
    case 0x13:
        G32(0x1013a008) = G32(0x1013a008) == 0;
        break;
    case 0x14:
        if ((U16(W14K_PLAYER(), 0x1c) & 2) == 0) {
            if (Sim_100448ff() == 0 && Sim_1009b2eb() == 0) {
                Sim_100448cd(1);
                Sim_10044a69(0, 1);
            } else {
                Sim_1009b2fc();
                Sim_100448cd(0);
                *GBYTES(0x1015fdba) = 1;
                *GBYTES(0x1015fdbd) = 1;
                *GBYTES(0x1015fdd0) = 1;
            }
        }
        break;
    case 0x19:
        W14K_SOLO(Sim_10010951(0));
        break;
    case 0x1a:
        W14K_SOLO(Sim_10010951(1));
        break;
    case 0x1b:
        W14K_SOLO(Sim_10010951(2));
        break;
    case 0x1c:
        W14K_SOLO(Sim_10010a73());
        break;
    case 0x1d:
        W14K_SOLO(Sim_10010b0b());
        break;
    case 0x1e:
    case 0x22:
        W14K_SOLO(Sim_10010b38());
        break;
    case 0x1f:
        W14K_SOLO(Sim_10010b65());
        break;
    case 0x20:
        W14K_SOLO(Sim_10010b92());
        break;
    case 0x21:
        W14K_SOLO(Sim_10010aa0());
        break;
    case 0x23:
        W14K_SOLO(Sim_10010acd());
        break;
    case 0x24:
        W14K_SOLO(Sim_10010aec());
        break;
    case 0x25:
        if (Sim_1002590c() != 0)
            Sim_100448cd(3);
        break;
    case 0x28:
        G32(0x1015f1e0) = 1;
        break;
    case 0x29:
        *GBYTES(0x1015fd8d) = 1;
        break;
    case 0x2a:
        G32(0x1015e550) = G32(0x1015e550) == 0;
        break;
    case 0x2b:
        if (G32(0x1015d1ac) != 4) {
            GS32(0x10144fa0) = (int32_t)(G32(0x10144fa0) + 1) % 3;
            if (G32(0x10144fa0) == 1) {
                Sim_10077fff();
                Sim_10078177();
            } else if (G32(0x10144fa0) == 2 && Sim_10077ecd() == 0) {
                G32(0x10144fa0) = 0;
            }
            G32(0x1017e5e0) = 0;
            W14K_Sound(0x1013b784, 8);
        }
        break;
    case 0x2c:
        G32(0x10179cbc) = 1;
        break;
    case 0x2f:
        *GBYTES(0x1015fdd0) = 1;
        break;
    case 0x32:
    case 0x33:
    case 0x34:
    case 0x35:
    case 0x36:
    case 0x37:
    case 0x38:
    case 0x39:
    case 0x3a:
    case 0x3b:
        throttle = (int32_t)cmd - 0x32;
        break;
    case 0x3c:
        *GBYTES(0x1015fd78) = 1;
        break;
    case 0x3d:
        *GBYTES(0x1015fd79) = 1;
        break;
    case 0x3e:
        *GBYTES(0x1015fd7a) = 1;
        break;
    case 0x3f:
        Sim_100ca96a();
        break;
    case 0x40:
        *GBYTES(0x1015fd69) = 1;
        break;
    case 0x41:
        *GBYTES(0x1015fd6a) = 1;
        break;
    case 0x42:
        *GBYTES(0x1015fd6b) = 1;
        break;
    case 0x43:
        *GBYTES(0x1015fd7b) = 1;
        break;
    case 0x44:
        *GBYTES(0x1015fd7c) = 1;
        break;
    case 0x45:
        *GBYTES(0x1015fd7d) = 1;
        break;
    case 0x46:
        Sim_1009b2b4();
        break;
    case 0x47:
        Sim_1009b3a1(1);
        break;
    case 0x48:
        Sim_1009b3a1(2);
        break;
    case 0x49:
        Sim_1009b3a1(0);
        break;
    case 0x4a:
        if (U32(unit, 0xbb3) == 0)
            FlashText_Start(2);
        else
            Sim_1009b362();
        break;
    case 0x4b:
        Sim_10074bfa(5);
        break;
    case 0x4c:
        Sim_10074c3d(6);
        break;
    case 0x53:
        G32(0x10144fa0) = 0;
        Sim_1000ebfb(SIM_PTR(const char, 0x1013b874), 0, 0, 0, 0x32);
        G32(0x1016551a) = 1;
        break;
    case 0x54:
        if (W14K_REPORTS()) {
            if (G32(0x101452d0) == 0) {
                G32(0x101452d0) = 1;
                W14K_Message(0x101c45b0, 0x16a);
            } else {
                G32(0x101452d0) = 0;
                W14K_Message(0x101c45b4, 0x16a);
            }
        }
        break;
    case 0x55:
        G32(0x10179cd8) = U32(unit, 0x4f0) == 3 ? 1 : 0xffffffffu;
        break;
    case 0x56:
        if (U32(unit, 0x4f0) != 7)
            G32(0x10179cd8) = U32(unit, 0x4f0) == 3 ? 1 : 0xffffffffu;
        break;
    case 0x57:
        *GBYTES(0x1015fd91) = (uint8_t)(1 - (*GBYTES(0x1015fd91) != 0));
        break;
    case 0x58:
        G32(0x1015f1d8) = 1;
        break;
    case 0x59:
        W14K_Cycle(0x1017e5e0);
        break;
    case 0x5a:
        W14K_Cycle(0x10151d54);
        break;
    case 0x5b:
        *GBYTES(0x1015fd8e) ^= 1;
        break;
    case 0x5c:
        Sim_10098e62();
        break;
    case 0x5d:
        if (U32(unit, 0xc3d) == 0) {
            if (G32(0x1015ec58) == 0) {
                group = (uint32_t)Sim_10015c61(1);
            } else {
                net = 1;
                group = (uint32_t)Sim_10015c61(0) == G32(0x1017b8c8);
                if (group != 0)
                    group = (uint32_t)Sim_10015c61(1);
            }
            W14K_Order(group, 0x1013b79c, 0x1013b7a8, 0x1013b7b4, 1, net);
        }
        break;
    case 0x5e:
        W14K_LanceOrder(unit, 2, 2, 2, 1, 0x1013b7c0, 0x1013b7cc, 0x1013b7d8);
        break;
    case 0x5f:
        if (U32(unit, 0xc3d) == 0) {
            Sim_1000ebfb(SIM_PTR(const char, 0x1013b790), 0, 0, 0, 0x32);
            if (G32(0x1015ec58) == 0)
                G32(0x1017e5ec) = (uint32_t)Sim_10074c3d(3);
            else
                G32(0x10151d50) = 0xffffffffu;
        }
        break;
    case 0x60:
        W14K_LanceOrder(unit, 3, 7, 7, 2, 0x1013b7e4, 0x1013b7f0, 0x1013b7fc);
        break;
    case 0x61:
        W14K_LanceOrder(unit, 4, 8, 8, 7, 0x1013b808, 0x1013b814, 0x1013b820);
        break;
    case 0x62:
        W14K_LanceOrder(unit, 5, 9, 9, 8, 0x1013b82c, 0x1013b838, 0x1013b844);
        break;
    case 0x63:
        W14K_LanceOrder(unit, 6, 10, 10, 9, 0x1013b850, 0x1013b85c, 0x1013b868);
        break;
    case 0x64:
        break;
    case 0x65:
        *GBYTES(0x1015fd7e) = 1;
        break;
    case 0x66:
        *GBYTES(0x1015fd65) = 1;
        break;
    case 0x67:
        *GBYTES(0x1015fd66) = 1;
        break;
    case 0x68:
        *GBYTES(0x1015fd6c) = 1;
        break;
    case 0x69:
        *GBYTES(0x1015fd6d) = 1;
        break;
    case 0x6a:
        *GBYTES(0x1015fd6e) = 1;
        break;
    case 0x6c:
        *GBYTES(0x1015fd74) = 1;
        break;
    case 0x6d:
        *GBYTES(0x1015fd70) = 1;
        break;
    case 0x6e:
        *GBYTES(0x1015fd71) = 1;
        break;
    case 0x6f:
        *GBYTES(0x1015fd6f) = 1;
        break;
    case 0x70:
        *GBYTES(0x1015fd75) = 1;
        break;
    case 0x71:
        *GBYTES(0x1015fd77) = 1;
        break;
    case 0x72:
        G32(0x1015dd10) = 1;
        break;
    case 0x74:
        *GBYTES(0x1015fd84) = 1;
        break;
    case 0x75:
        G32(0x1017b930) = 1;
        break;
    case 0x76:
        G32(0x1017b8b8) = 1;
        G32(0x1017b8bc) += 2;
        break;
    case 0x78:
        G32(0x1015f204) = 1;
        break;
    case 0x89:
        Sim_1006abf6(0xfffffffeu, BYTES(U32(W14K_PLAYER(), 0x28)), 5);
        break;
    case 0x8a:
        Sim_1006abf6(0xfffffffeu, BYTES(U32(W14K_PLAYER(), 0x28)), 6);
        break;
    case 0xb0:
        if (G32(0x1015f1ec) != 0) {
            G32(0x1015dc8c) = G32(0x1015dc8c) == 0;
            if (G32(0x1015dc8c) == 0)
                W14K_Message(0x101c45a8, 0x16a);
            else
                W14K_Message(0x101c45ac, 0xb50);
        }
        break;
    case 0xb5:
        Sim_100d6392(0);
        break;
    case 0xb6:
        Sim_100d6392(1);
        break;
    case 0xb7:
        Sim_100d6392(2);
        break;
    case 0xb8:
        *GBYTES(0x1015fd64) = 1;
        break;
    case 0xb9:
        *GBYTES(0x1015fd60) = 1;
        break;
    case 0xba:
        *GBYTES(0x1015fd61) = 1;
        break;
    case 0xbb:
        *GBYTES(0x1015fd62) = 1;
        break;
    case 0xbc:
        *GBYTES(0x1015fd63) = 1;
        break;
    case 0xc4:
        if (U32(unit, 0xbab) == 0)
            FlashText_Start(0);
        else
            Sim_1003cbb3(0, G32(0x109b9e08) == 0);
        break;
    case 0xc5:
        if (U32(unit, 0xbaf) == 0) {
            FlashText_Start(1);
        } else if (G32(0x1014cb1c) == 1 || !W14K_REPORTS()) {
            /*
             * 1.2 (0x1003b3ba): the renderer's fog state as (1, 0x200000), 1.0's code 2 (1.0 passed (0x200000, 1),
             * a code its renderers reject), and in its 3DFX / PowerVR mode state 0xe (1.0's 0xf) on.
             */
            if (G32(0x1017d108) != 0)
                (*SIM_PTR(W14kSoundFn, 0x109e3d40))(2, 0x200000);
            G32(0x1014cb1c) = 0;
            if (g_w33Voodoo != 0)
                (*SIM_PTR(W14kSoundFn, 0x109e3d40))(0xf, 1);
        } else {
            G32(0x1014cb1c) = 1;
            G32(0x1014cb20) = 0;
            Sim_100203cf(1, 1);
#ifdef W14KV_MUTATE /* the checker's negative check only: 1.0's call */
            (*SIM_PTR(W14kSoundFn, 0x109e3d40))(0x200000, 0);
#else
            (*SIM_PTR(W14kSoundFn, 0x109e3d40))(2, 0);
#endif
            if (g_w33Voodoo != 0)
                (*SIM_PTR(W14kSoundFn, 0x109e3d40))(0xf, 0);
        }
        break;
    case 0xc6:
        Sim_10019670();
        break;
    case 0xc7:
        Sim_100d6392(3);
        break;
    case 0xc9:
        Sim_100d6392(4);
        break;
    case 0xcb:
        *GBYTES(0x1015fd60) = 1;
        *GBYTES(0x1015fd64) = 1;
        break;
    case 0xcc:
        Sim_100b8ff9(SIM_PTR(const char, 0x1013b880));
        break;
    default:
        if (G32(0x1015f1e8) != 0)
            Sim_1008b922(cmd & 0xffff);
        break;
    }
    if (throttle != -1) {
        *SIM_PTR(float, 0x1015fd40) = (float)((double)throttle * (double)*SIM_PTR(float, 0x101115f0));
        *GBYTES(0x1015fd5d) = 1;
    }
}
