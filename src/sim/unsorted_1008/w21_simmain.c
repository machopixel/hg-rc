#include <string.h>

#include "w21.h"
#include "../render/w31.h"
#include "../util/access.h"
#include "../../platform/plat.h"
#include "../../platform/sim_platform.h"

#ifdef _MSC_VER
#include <excpt.h>
/* Each call pushes its arguments and pops them right after, as in the original (see w21.h). */
#pragma optimize("", off)
#define W21_TRY __try
#define W21_FINALLY __finally
#define W21_ABNORMAL() AbnormalTermination()
#else
#define W21_TRY
#define W21_FINALLY
#define W21_ABNORMAL() 0 /* only the MSVC build has the __try; elsewhere no exception unwinds through it */
#endif

typedef uint32_t(__cdecl *W21F0)(void);
typedef uint32_t(__cdecl *W21F1)(uint32_t);
typedef uint32_t(__cdecl *W21F2)(uint32_t, uint32_t);
typedef uint32_t(__cdecl *W21F3)(uint32_t, uint32_t, uint32_t);
typedef uint32_t(__cdecl *W21F4)(uint32_t, uint32_t, uint32_t, uint32_t);
typedef uint32_t(__cdecl *W21F5)(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
typedef uint32_t(__cdecl *W21F6)(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
typedef uint32_t(__cdecl *W21F7)(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);

/* A function of the original DLL by its address (converted ones are reached through their hooks). */
#define C0(a) SIM_FN(W21F0, a)()
#define C1(a, x) SIM_FN(W21F1, a)((uint32_t)(x))
#define C2(a, x, y) SIM_FN(W21F2, a)((uint32_t)(x), (uint32_t)(y))
#define C3(a, x, y, z) SIM_FN(W21F3, a)((uint32_t)(x), (uint32_t)(y), (uint32_t)(z))
#define C4(a, x, y, z, w) SIM_FN(W21F4, a)((uint32_t)(x), (uint32_t)(y), (uint32_t)(z), (uint32_t)(w))
#define C5(a, x, y, z, w, v) SIM_FN(W21F5, a)((uint32_t)(x), (uint32_t)(y), (uint32_t)(z), (uint32_t)(w), (uint32_t)(v))
#define C6(a, x, y, z, w, v, u) \
    SIM_FN(W21F6, a)((uint32_t)(x), (uint32_t)(y), (uint32_t)(z), (uint32_t)(w), (uint32_t)(v), (uint32_t)(u))
#define C7(a, x, y, z, w, v, u, t) \
    SIM_FN(W21F7, a)((uint32_t)(x), (uint32_t)(y), (uint32_t)(z), (uint32_t)(w), (uint32_t)(v), (uint32_t)(u), \
                     (uint32_t)(t))
/* a function pointer kept in DLL data */
#define CI0(slot) ((W21F0)(uintptr_t)G32(slot))()
#define CI1(slot, x) ((W21F1)(uintptr_t)G32(slot))((uint32_t)(x))
#define P(p) ((uint32_t)(uintptr_t)(p))
#define S(addr) P(SIM_PTR(char, addr))
#define NETPOLL() C0(0x1003e291)
#define LOGSTEP(text) C1(0x10083495, S(text))

/* the display driver's flip test through slot +0x20 of [0x1015e688] (-1 without [0x1017b920]) */
static int32_t W21_DriverReady(void)
{
    if (G32(0x1017b920) != 0)
        return (int32_t)((W21F0)(uintptr_t)U32(G32(0x1015e688), 0x20))();
    return -1;
}

/* the six heaps, the first one sized by the physical memory; system error 9 when even its smallest size fails */
static void W21_Heaps(const uint8_t *memStatus)
{
    if (U32(memStatus, 8) > 0x1e84800)
        G32(0x1017b900) = SimPlat_HeapCreate(1, 0x3e8000, 0);
    else if (U32(memStatus, 8) > 0x16e3600)
        G32(0x1017b900) = SimPlat_HeapCreate(1, 0x1f4000, 0);
    else
        G32(0x1017b900) = SimPlat_HeapCreate(1, 0xfa000, 0);
    G32(0x1017b904) = SimPlat_HeapCreate(1, 0x177000, 0);
    G32(0x1017b908) = SimPlat_HeapCreate(1, 0x177000, 0);
    G32(0x1017b90c) = SimPlat_HeapCreate(1, 0x36b000, 0);
    G32(0x1017b8fc) = SimPlat_HeapCreate(1, 0xaa000, 0);
    G32(0x1017b910) = SimPlat_HeapCreate(1, 0x20000, 0);
    if (G32(0x1017b900) == 0) {
        G32(0x1017b900) = SimPlat_HeapCreate(1, 0xfa000, 0);
        if (G32(0x1017b900) == 0)
            C2(0x100bc280, 9, G32(0x101c514c));
    }
    G32(0x1017b8f8) = G32(0x1017b900);
    /* round 31: 1.2 (0x1006fcfc) notes the time here; its texture preload stops 30 s after (Vulkan only) */
    if (g_w31Tex)
        g_w31TexStart = Plat_TicksMs();
}

// FUNCTION: HGSIM 0x10084feb
uint32_t __stdcall SimMain(uint32_t instance, uint32_t unused1, char *cmdline, uint8_t *net, uint32_t unused2,
                           uint32_t window)
{
    uint8_t systemTime[0x10];   /* [ebp-0x30] */
    uint32_t drv[0x14];         /* [ebp-0x88], the display driver's start record */
    uint32_t opts[2];           /* [ebp-0x90] / [ebp-0x8c] */
    char mission[0x48];         /* [ebp-0xd8] */
    uint8_t memStatus[0x20];    /* [ebp-0x108] */
    uint8_t msg[0x1c];          /* [ebp-0x130] */
    uint32_t dc, firstQuit = 0, zero34 = 0, zeroDc = 0, fd, i, j, hasDc, *res;
    int32_t cfg, ret;
    const char *name;

    (void)unused1;
    (void)unused2;
    G32(0x1017c8c4) = 1;
    C0(0x1005f6c6);
    G32(0x1017b8f0) = window;
    G32(0x1017b8f4) = instance;
    G32(0x1015e9c4) = (uint32_t)SimPlat_GetSystemMetrics(0);
    G32(0x1015e9c8) = (uint32_t)SimPlat_GetSystemMetrics(1);
    G32(0x1017b958) = SimPlat_GlobalAlloc(0, 0x249f0);
    G32(0x1017b95c) = SimPlat_GlobalSize(G32(0x1017b958));
    SimPlat_GetSystemTime(systemTime);
    C3(0x100fcab0, P(memStatus), 0, 0x20);
    U32(memStatus, 0) = 0x20;
    SimPlat_GlobalMemoryStatus(memStatus);
    W21_Heaps(memStatus);
    opts[1] = 1;
    opts[0] = 0;
    if (C1(0x10100130, S(0x1014b188)) != 0)
        C2(0x100fc690, S(0x1015f050), C1(0x10100130, S(0x1014b198)));
    SimPlat_SetEnvironmentVariable(SIM_PTR(const char, 0x1014b1ac), SIM_PTR(const char, 0x1014b1a8));
    if ((int32_t)C2(0x1004712a, S(0x1014b1c0), S(0x1015d1b8)) == -1) {
        if (G32(0x1015d1b8) == 0)
            C3(0x100bc280, 0x11, S(0x1014b1d8), S(0x1014b1cc));
        else
            memcpy((void *)(uintptr_t)G32(0x1015d1b8), SIM_PTR(uint8_t, 0x101321b0), 0x1b * 4);
    } else {
        memcpy(SIM_PTR(uint8_t, 0x101321b0), (void *)(uintptr_t)G32(0x1015d1b8), 0x1b * 4);
    }
    if (G32(0x1015d1b8) != 0) {
        C2(0x100988fc, zeroDc, U32(G32(0x1015d1b8), 0x14));
        C2(0x100989a1, zeroDc, U32(G32(0x1015d1b8), 0x18));
    }
    G32(0x1013b990) = U32(G32(0x1015d1b8), 0x44);
    G32(0x1013b98c) = G32(0x1013b990);
    G32(0x109e89e0) = 0;
    if ((int8_t)U8(G32(0x1015d1b8), 0x48) != 0) {
        G32(0x109e89e0) |= 1;
        if (C2(0x100ff1e0, G32(0x1015d1b8) + 0x48, S(0x1014b1dc)) == 0) {
            U8(GBYTES(0x109e89e4), 0) = 0;
        } else {
            C3(0x100fc7f0, S(0x109e89e4), G32(0x1015d1b8) + 0x48, 0xc);
            U8(GBYTES(0x109e89f0), 0) = 0;
        }
    }
    if (C3(0x1002ad50, P(cmdline), P(opts), P(mission)) == 0)
        return 0;
    if (G32(0x1017b954) != 0) {
        C2(0x100fc690, S(0x1017ed20), P(mission));
        C2(0x100fc6a0, S(0x1017ed20), S(0x1014b1e4));
    }
    if (G32(0x1017b940) != 0)
        C3(0x1003695b, S(0x1014b1ec), 0, 1);
    else if (G32(0x1017b944) != 0)
        C3(0x1003695b, S(0x1014b1f4), 0, 2);
    else
        C3(0x1003695b, S(0x1014b1fc), 0, 0);
    if (C0(0x100d1df9) != 0)
        C2(0x100bc280, 0x51, 0);
    if ((int8_t)C0(0x10099510) == 0)
        C3(0x100bc280, 0x53, G32(0x101c5148), 0);
    else
        SimPlat_GetSystemTime(systemTime);
    C1(0x10086d8d, S(0x109e89e4));
    if (G32(0x101668f4) != 0)
        C0(0x1005f8d0);
    res = (uint32_t *)(uintptr_t)C1(0x10036876, S(0x1014b204));
    if (G32(0x1017b940) != 0 && U32(res, 0x44) != 1)
        G32(0x1017b940) = 0;
    if (C0(0x10034c11) == 0)
        C2(0x100bc280, 0x50, 0);
    if (C2(0x100fc970, P(res) + 4, S(0x1014b210)) != 0)
        G32(0x1017d0ac) = 1;
    LOGSTEP(0x1014b218);
    C6(0x1003795b, 5, 0, S(0x109e3e20), 0xffffffffu, 0xffffffffu, 0);
    SimPlat_SendMessage(G32(0x1017b8f0), 0x41f, 0, 0);
    SimPlat_SendMessage(G32(0x1017b8f0), 0x1c, 1, 0);
    while (SimPlat_ShowCursor(0) >= 0)
        ;
    G32(0x1017b93c) = 0;
    if (G32(0x1015ec58) != 0) {
        if (net == 0)
            net = (uint8_t *)(uintptr_t)SimPlat_HeapAlloc(G32(0x1017b8f8), 9, 0x13c);
        U32(net, 0x16) = SimPlat_HeapAlloc(G32(0x1017b8f8), 9, 0x10);
        C3(0x100fcab0, U32(net, 0x16), 0xfa01, 0x10);
        if ((int8_t)U8(GBYTES(0x109e89fe), 0) == 0) {
            G32(0x1017b8c8) = 0;
            G32(0x1015ecd0) = 0;
            G32(0x1015dc84) = 0;
            U32(PTRTAB(G32(0x1017b8c8)), 0x14) = 0;
        } else {
            C1(0x10086ebf, P(net));
            C3(0x100fc7f0, P(net) + 0x1e, S(0x109e89fe), 0x100);
            U8(net, 0x11d) = 0;
            G32(0x1015ec58) = U32(net, 0x12);
        }
    }
    if (G32(0x1015ec58) != 0) {
        cfg = (int32_t)C2(0x10046fe5, S(0x1014b228), S(0x1017b8c4));
        if (cfg == -1 || G32(0x1017b8c4) == 0)
            C3(0x100bc280, 0x11, S(0x1014b240), S(0x1014b234));
    } else {
        cfg = (int32_t)C2(0x10046fe5, S(0x1014b244), S(0x1017b8c4));
        if (cfg == -1 || G32(0x1017b8c4) == 0)
            C3(0x100bc280, 0x11, S(0x1014b25c), S(0x1014b250));
    }
    LOGSTEP(0x1014b260);
    W21_TRY {
        C0(0x1002a8c2);
        LOGSTEP(0x1014b270);
        C1(0x1003dc19, P(net));
        LOGSTEP(0x1014b280);
        if (C0(0x1003e26a) == 0) {
            G32(0x1017b8b8) = 1;
            G32(0x1017b8bc) = 3;
        }
        NETPOLL();
        LOGSTEP(0x1014b298);
        C2(0x100b9eab, S(0x1014b2b8), S(0x1014b2ac));
        LOGSTEP(0x1014b2c4);
        dc = SimPlat_GetDC(G32(0x1017b8f0));
        hasDc = SimPlat_GetDeviceCaps(dc, 0x18) == -1 || G32(0x1015e9bc) == 1 ? 1 : 0;
        C1(0x100b82a0, hasDc);
        LOGSTEP(0x1014b2d4);
        NETPOLL();
        C1(0x100b6590, P(mission));
        LOGSTEP(0x1014b2e8);
        C0(0x1004b710);
        NETPOLL();
        LOGSTEP(0x1014b2fc);
        C1(0x1009c860, zero34);
        LOGSTEP(0x1014b30c);
        C1(0x1001f415, window);
        NETPOLL();
        LOGSTEP(0x1014b31c);
        C0(0x10034cca);
        NETPOLL();
        LOGSTEP(0x1014b32c);
        C0(0x1002abd6);
        LOGSTEP(0x1014b33c);
        C2(0x100e46d0, 0x80, 1);
        LOGSTEP(0x1014b35c);
        C0(0x10021ac0);
        LOGSTEP(0x1014b36c);
        C0(0x1002a1c1);
        LOGSTEP(0x1014b380);
        C0(0x1002986c);
        NETPOLL();
        C0(0x10058389);
        C0(0x100d7fc0);
        LOGSTEP(0x1014b390);
        C1(0x100e122f, P(mission));
        NETPOLL();
        LOGSTEP(0x1014b3a0);
        C0(0x1007e66c);
        NETPOLL();
        LOGSTEP(0x1014b3b8);
        C0(0x100ab920);
        LOGSTEP(0x1014b3c4);
        C0(0x1003c970);
        NETPOLL();
        LOGSTEP(0x1014b3d8);
        C0(0x100b418e);
        NETPOLL();
        LOGSTEP(0x1014b3ec);
        C0(0x100a774e);
        NETPOLL();
        C1(0x100b979f, G32(0x1014cedc));
        NETPOLL();
        LOGSTEP(0x1014b400);
        C0(0x100e1398);
        LOGSTEP(0x1014b414);
        C0(0x1004f0a6);
        LOGSTEP(0x1014b424);
        C0(0x10044420);
        LOGSTEP(0x1014b438);
        C0(0x1005502a);
        NETPOLL();
        C0(0x1001db6f);
        NETPOLL();
        C1(0x100a0063, P(mission));
        LOGSTEP(0x1014b448);
        C0(0x1007520b);
        LOGSTEP(0x1014b458);
        C1(0x10074b80, 1);
        C1(0x10074b80, 2);
        C1(0x10074b80, 3);
        C1(0x10074b80, 5);
        C1(0x10074b80, 6);
        C1(0x10074b80, 4);
        C1(0x10074b80, 7);
        C1(0x10074b80, 8);
        C1(0x10074b80, 9);
        C1(0x10074b80, 10);
        LOGSTEP(0x1014b46c);
        for (i = 0; (int32_t)i < GS32(0x104737e0); i++) {
            NETPOLL();
            C2(0x10079b68, S(0x10473800) + i * 0x4fc3au, i);
        }
        LOGSTEP(0x1014b480);
        C0(0x100b388a);
        NETPOLL();
        if (G32(0x1017b954) != 0) {
            G32(0x1017ed8c) = C2(0x100fd850, S(0x1017ed20), S(0x1014b494));
            C3(0x100fd7e0, G32(0x1017ed8c), S(0x1014b498), G32(0x1015cff4));
            C2(0x100fd7e0, G32(0x1017ed8c), S(0x1014b4bc));
            for (i = 0; (int32_t)i < 0x200; i++) {
                NETPOLL();
                G32(0x1015dafc) = (uint32_t)(uintptr_t)W31_NAME(i, 0); /* round 31: the moved table */
                for (j = 0; (int32_t)j < 0x20; j++) {
                    if (*(const int8_t *)(uintptr_t)G32(0x1015dafc) == 0) /* a pointer into our table now */
                        break;
                    G32(0x1017b950) += 1;
                    C3(0x100fd7e0, G32(0x1017ed8c), S(0x1014b4d0), G32(0x1015dafc));
                    G32(0x1015dafc) += W31_FRAME_SIZE;
                }
            }
            C3(0x100fd7e0, G32(0x1017ed8c), S(0x1014b4e4), G32(0x1017b950));
            G32(0x1015dafc) = 0;
            C1(0x100fd760, G32(0x1017ed8c));
            G32(0x1017ed8c) = 0;
        }
        LOGSTEP(0x1014b504);
        C0(0x100abcb3);
        LOGSTEP(0x1014b51c);
        C0(0x10098be0);
        LOGSTEP(0x1014b52c);
        C0(0x10039241);
        LOGSTEP(0x1014b540);
        if (U32(G32(0x1015d1b8), 0x68) != 0) {
            if (C2(0x10045d70, window, 0) == 0)
                G32(0x1015ef3c) = 1;
            NETPOLL();
        }
        C0(0x10025d93);
        LOGSTEP(0x1014b550);
        C0(0x10033aeb);
        NETPOLL();
        LOGSTEP(0x1014b564);
        C0(0x1000c894);
        LOGSTEP(0x1014b574);
        C0(0x10029f50);
        NETPOLL();
        LOGSTEP(0x1014b58c);
        if (G32(0x1015ec58) != 0)
            C0(0x10042a11);
        else
            C0(0x10010f70);
        LOGSTEP(0x1014b598);
        C0(0x10034dc2);
        NETPOLL();
        LOGSTEP(0x1014b5a8);
        C0(0x10098710);
        if (G32(0x1015f1e8) != 0) {
            LOGSTEP(0x1014b5bc);
            C2(0x100085b7, P(mission), 0);
        }
        NETPOLL();
        /* 1.2 (0x1006fae1, round 32) sets up the effect pool after the display and renderer set-up below, once the
           3D mode is known (its particle layout and the particle types' textures depend on it) */
        if (!g_w31Tex)
            C0(0x100908b3);
        C0(0x1001db6a);
        NETPOLL();
        C0(0x1008988f);
        C0(0x100b8938);
        if (G32(0x1017b940) != 0) {
            G32(0x1013a010) = 0;
            G32(0x1013a00c) = 0;
        }
        if (G32(0x1017b944) != 0) {
            G32(0x1017b940) = 1;
            G32(0x1013a010) = 0;
            G32(0x1013a00c) = 6;
            G32(0x1015e688) = S(0x1014b838);
        }
        if (C6(0x1003795b, G32(0x1013a00c), G32(0x1013a010), S(0x109e3e20), G32(0x1017b918), G32(0x1017b91c), 0) == 0)
            C2(0x100bc280, 0x50, G32(0x101c5140));
        if (G32(0x1017b918) == 0x280 && G32(0x1017b91c) == 0x1e0 && G32(0x1017b940) != 0) {
            G32(0x109e3dc4) = U32(G32(0x109e3e00), 4) + 1;
            G32(0x109e3dc8) = U32(G32(0x109e3e00), 8) + 1;
            U32(G32(0x109e3e00), 8) = U32(G32(0x109e3e00), 8) - 1;
            G32(0x109e3dd0) = 0;
            G32(0x109e3dd4) = 0;
            G32(0x109e3dd8) = G32(0x109e3e0c);
            G32(0x109e3ddc) = G32(0x109e3e10);
            G32(0x109e3dc0) = 1;
            memset(drv, 0, sizeof(drv));    /* the fields the original never sets: see w21.h */
            drv[0x13] = G32(0x1017b8f0);    /* -0x3c */
            drv[0] = S(0x109e3dc0);          /* -0x88 */
            drv[8] = G32(0x1017b390);       /* -0x68 */
            drv[0xb] = G32(0x1017b39c);     /* -0x5c */
            drv[0xf] = 0x280;               /* -0x4c */
            drv[0x10] = 0x1e0;              /* -0x48 */
            drv[0x11] = 0xffff;             /* -0x44 */
            drv[9] = 0;                     /* -0x64 */
            drv[0xa] = 0x200;               /* -0x60 */
            drv[1] = 8;                     /* -0x84 */
            drv[2] = 0x80;                  /* -0x80 */
            if (U32(G32(0x1015d1b8), 0x3c) != 0)
                drv[2] |= 0x200;
            if (G32(0x1017d108) != 0) {
                drv[2] |= 2;
                drv[4] = 3;                 /* -0x78 */
                drv[5] = (G32(0x1014c9ec) << 24) | (G32(0x1017d0f0) << 16) | (G32(0x1017d0f4) << 8) | G32(0x1017d0f8);
                drv[6] = 0x3f800000;        /* -0x70 */
                drv[7] = 0x48f42400;        /* -0x6c */
            }
            CI1(0x109e3d28, P(drv));
        }
        if (g_w31Tex)
            C0(0x100908b3);
        if (SimPlat_GetDeviceCaps(dc, 0x18) == -1 || G32(0x1015e9bc) == 1) {
            LOGSTEP(0x1014b5d0);
            C1(0x100d3877, 0);
            NETPOLL();
        } else {
            name = SIM_PTR(const char, 0x10185720);
            if ((int8_t)name[0] != 0) {
                fd = C3(0x100ba7d5, P(name), G32(0x1014ced4), 0);
                if (fd != 0)
                    ((W21F4)(uintptr_t)U32(G32(0x1015e688), 0x14))(0, 0x100, fd, 1);
                NETPOLL();
            }
        }
        if (G32(0x1017692c) == 0) {
            G32(0x1017692c) = 1;
            C0(0x10062386);
        }
        G32(0x1017b924) = 1;
        SimPlat_ReleaseDC(G32(0x1017b8f0), dc);
        if (G32(0x1017b958) != 0)
            SimPlat_GlobalFree(G32(0x1017b958));
        G32(0x1017b958) = 0;
        G32(0x1017b95c) = 0;
        if (G32(0x1017b910) != 0)
            SimPlat_HeapDestroy(G32(0x1017b910));
        G32(0x1017b910) = 0;
        if (G32(0x1015e694) == 0)
            G32(0x1017b94c) |= 2;
        U8(GBYTES(0x101653fd), 0) = 1;
        G32(0x101655ac) = 0xffffffffu;
        G32(0x1017b934) = 1;
        if (G32(0x1017b940) != 0) {
            C0(0x100a2a66);
            NETPOLL();
            C0(0x10006e52);
            NETPOLL();
        }
        if (G32(0x1017b940) != 0 && G32(0x1017b944) == 0) {
            C5(0x1008181e, 0, 0, G32(0x1017b918), G32(0x1017b91c), 0);
            C5(0x1008181e, 0, 0, G32(0x1017b918), G32(0x1017b91c), 0);
        }
        LOGSTEP(0x1014b5e4);
        C0(0x10041e61);
        while (GS32(0x1017b8bc) < 3) {
            C0(0x10086b99);
            if (G32(0x1017b928) == 0)
                continue;
            if (G32(0x1017b94c) == 3) {
                LOGSTEP(0x1014b5f8);
                G32(0x1017b94c) |= 0x80000000u;
                LOGSTEP(0x1014b610);
                C0(0x1001f26e);
                C2(0x100e46d0, 0x80, 0);
                G32(0x1015f208) = 0;
            } else if (G32(0x1015ec58) != 0 && (G32(0x1017b94c) & 0x80000000u) == 0) {
                if (G32(0x1017b8cc) == 0)
                    G32(0x1017b8cc) = G32(0x1015dc7c) + 0xb5;
                else if (GS32(0x1015dc7c) > GS32(0x1017b8cc))
                    G32(0x1015f208) = 3;
            }
            C0(0x1002a7e0);
            C0(0x1002a983);
            C0(0x10055048);
            C0(0x10075371);
            C3(0x10048f02, 0, 0, 0);
            C1(0x100cc8c5, S(0x1017b8c0));
            NETPOLL();
            C0(0x1002a086);
            C0(0x10029fba);
            C0(0x1001c693);
            C0(0x1002a019);
            C0(0x10021d65);
            C0(0x1002528f);
            C0(0x10090e23);
            C0(0x100445d6);
            C0(0x10029067);
            C0(0x100d82f6);
            C0(0x100ac001);
            C0(0x10045db9);
            C0(0x10058493);
            if (G32(0x1015e694) != 0 && G32(0x1017b940) == 0) {
                while (G32(0x1015e694) != 0) {
                    if (U32(G32(0x1015e688), 0) == 0)
                        C5(0x1008181e, 0, 0, G32(0x1017b918), G32(0x1017b91c), 0xf0);
                    if (W21_DriverReady() == 0) {
                        CI0(0x1014cb3c);
                        C0(0x1002a0ca);
                        C0(0x10075585);
                        C0(0x1007e0ff);
                    }
                    if (U32(G32(0x1015e688), 0) == 0)
                        C5(0x1008181e, 0, 0, G32(0x1017b918), G32(0x1017b91c), 0xf0);
                    C0(0x10035478);
                    C0(0x10037cab);
                    C0(0x100d3491);
                    C0(0x100d33ec);
                }
            }
            if ((G32(0x1017b94c) & 0x80000000u) != 0)
                C0(0x100b388a);
            C0(0x10006a2f);
            if (G32(0x1017b940) != 0 || W21_DriverReady() == 0) {
                G32(0x1017b948) = 1;
                CI0(0x1014cb3c);
                if (G32(0x1017b940) == 0 || G32(0x1017b948) == 0 || W21_DriverReady() == 0) {
                    G32(0x1017b948) = 0;
                    C0(0x10089bad);
                    C0(0x1002a0ca);
                    C0(0x1007e0ff);
                    C0(0x10075585);
                    if (G32(0x1017b940) != 0) {
                        if (G32(0x1017b940) != 0)
                            C7(0x100ef727, S(0x109e3e00), 0, 0x1df, 0x27f, 0x1df, 0, U16(GBYTES(0x10165bc0), 0));
                        else
                            C7(0x100e86a1, S(0x109e3e00), 0, 0x1df, 0x27f, 0x1df, 0, 0xf0);
                        if (G32(0x1017b940) != 0)
                            C7(0x100ef727, S(0x109e3e00), 0x27f, 0, 0x27f, 0x1df, 0, U16(GBYTES(0x10165bc0), 0));
                        else
                            C7(0x100e86a1, S(0x109e3e00), 0x27f, 0, 0x27f, 0x1df, 0, 0xf0);
                    }
                    if (G32(0x1017b92c) != 0 && G32(0x1017b930) != 0)
                        C0(0x1008b6a0);
                    if (G32(0x1015f204) != 0) {
                        G32(0x1015f204) = 0;
                        C2(0x100e46d0, 0x80, 1);
                        C0(0x100473e4);
                        C2(0x100e46d0, 0x80, 0);
                        C2(0x100fc780, S(0x1015e9e0), G32(0x101c45a4));
                        C4(0x1007df6c, S(0x1015e9e0), G32(0x109b98c0), 0x16a, 0x32);
                    }
                }
            }
            C0(0x10035478);
            G32(0x1017b948) = 1;
            C0(0x100352d5);
            CI0(0x109e3d34);
            if ((G32(0x1017b94c) & 0x80000000u) != 0) {
                C0(0x1001f454);
                C0(0x1001fb6d);
            }
            C0(0x1003ca04);
            C0(0x1007c42f);
            C0(0x1001f3ac);
            C0(0x10086c32);
            if (G32(0x1017b8b8) != 0 && firstQuit == 0) {
                G32(0x1017b8bc) += 1;
                firstQuit = 1;
                G32(0x1017b924) = 0;
            }
            G32(0x1017b94c) |= 2;
        }
        NETPOLL();
        C1(0x10061bc5, U8(GBYTES(0x101653fd), 0) & 4);
        if (W21_DriverReady() == 0) {
            if (G32(0x1017b940) != 0)
                C2(0x100f0865, S(0x109e3e00), 0);
            else
                C2(0x100eb070, S(0x109e3e00), 0xf0);
        }
        NETPOLL();
        C0(0x10035478);
        while (SimPlat_PeekMessage(msg, 0, 0, 0, 1) != 0)
            NETPOLL();
        SimPlat_SendMessage(G32(0x1017b8f0), 0x41e, 0, 0);
        C0(0x10033c24);
        C0(0x10025dac);
        C0(0x1002a10e);
        NETPOLL();
        C0(0x1001f47d);
        C0(0x10074c8f);
        NETPOLL();
        C0(0x1002ab26);
        C0(0x1001dcff);
        NETPOLL();
        C0(0x100ba5d1);
        C0(0x100354ff);
        NETPOLL();
        C0(0x100556c2);
        NETPOLL();
        C0(0x1003693c);
        NETPOLL();
        C0(0x1003924b);
        C0(0x10045e19);
        NETPOLL();
        if (G32(0x101668f4) != 0)
            C0(0x1005f949);
        LOGSTEP(0x1014b630);
        NETPOLL();
        C0(0x1007c884);
        C0(0x1007c89b);
        NETPOLL();
    }
    W21_FINALLY {
        if (W21_ABNORMAL() != 0 && G32(0x1015dca0) != 0) {
            SimPlat_SendMessage(G32(0x1017b8f0), 0x41e, 0, 0);
            SimPlat_MessageBox(0, SIM_PTR(const char, 0x1014b658), SIM_PTR(const char, 0x1014b64c), 0x10);
        }
    }
    NETPOLL();
    if (G32(0x1017b910) != 0)
        SimPlat_HeapDestroy(G32(0x1017b910));
    if (G32(0x1017b900) != 0)
        SimPlat_HeapDestroy(G32(0x1017b900));
    if (G32(0x1017b904) != 0)
        SimPlat_HeapDestroy(G32(0x1017b904));
    if (G32(0x1017b908) != 0)
        SimPlat_HeapDestroy(G32(0x1017b908));
    if (G32(0x1017b8fc) != 0)
        SimPlat_HeapDestroy(G32(0x1017b8fc));
    if (G32(0x1017b90c) != 0)
        SimPlat_HeapDestroy(G32(0x1017b90c));
    NETPOLL();
    G32(0x1017b8f8) = 0;
    G32(0x1017b910) = 0;
    G32(0x1017b900) = 0;
    G32(0x1017b904) = 0;
    G32(0x1017b908) = 0;
    G32(0x1017b8fc) = 0;
    G32(0x1017b90c) = 0;
    G32(0x1017b914) = 0;
    SimPlat_ShowCursor(1);
    NETPOLL();
    C0(0x1003edda);
    ret = G32(0x1015ebf8) != 0 ? 0xff : 0;
    return (uint32_t)ret;
}
