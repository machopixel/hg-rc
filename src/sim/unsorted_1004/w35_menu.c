#include "w35.h"
#include "fns.h"
#include "w3p.h"
#include "w6e.h"
#include "w14.h"
#include "../util/access.h"
#include "../view/view.h"
#include "../unsorted_1007/w2e.h"
#include "../unsorted_1001/w6d.h"
#include "../unsorted_1003/w6e.h"
#include "../unsorted_1005/w2k.h"
#include "../unsorted_1006/w8w.h"
#include "../unsorted_1007/fns.h"
#include "../unsorted_1007/w3r.h"

/* still original: the key test 0x10048c84, the C library's sprintf, the respawn 0x1009d200 (left out of the
   conversion; reads uninitialised stack) */
#define W35_CALL_10048c84 SIM_FN(int(__cdecl *)(uint32_t), 0x10048c84)
#define W35_SPRINTF SIM_FN(int(__cdecl *)(char *, const char *, ...), 0x100fc780)
#define W35_CALL_1009d200 SIM_FN(int(__cdecl *)(uint32_t, uint32_t, uint32_t, uint32_t), 0x1009d200)

#define W35_SELF() G32(0x1017b8c8)
#define W35_CFG(o) U8(G32(0x1017b8c4), (o))
/* the player record's +0x168 points at the byte that counts the continues left */
#define W35_LIVES() (*BYTES(U32(PTRTAB(W35_SELF()), 0x168)))

static void W35_Quit(void)
{
    G32(0x1017b8b8) = 1;
    G32(0x1017b8bc) = 0x29a;
}

/* the player's gear stops (+0xc3d = 0) and its record goes to state 2 */
static void W35_Stop(void)
{
    uint8_t *unit = BYTES(U32(PTRTAB(W35_SELF()), 0x28));

    U32(unit, 0xc3d) = 0;
    Sim_1006abf6((uint32_t)(int32_t)(int16_t)U32(U32(unit, 0), 0x150), unit, 2);
}

// FUNCTION: HGSIM12 0x10089c60
void __cdecl Sim12_10089c60(void)
{
    G32(0x1015d1ac) = 0;
    G32(0x1015d1cc) = 1;
    G32(0x1015d098) = 0;
}

// FUNCTION: HGSIM 0x10048f02
void __cdecl Sim_10048f02(uint32_t a, uint32_t b, uint32_t c)
{
    char text[0x20];
    uint16_t key;

    (void)a, (void)b, (void)c;
    if (G32(0x1017b92c) != 0)
        return;
    if (GS32(0x1015f1d4) > 0 && GS32(0x1015dc78) > GS32(0x1015f1d4)) {
        Sim_1001fee5(1);
        G32(0x1015f1d4) = 0;
    }
    if (G32(0x1015f1d0) == 0 && G32(0x1015f1e4) == 0) {
        if (G32(0x1015f1fc) != 0 || G32(0x1015ebf8) != 0) {
            G32(0x1015f208) = 0;
            G32(0x1015f1e4) = 1;
            G32(0x1015f1d4) = G32(0x1015dc78) + 0x108;
            G32(0x1015f1d0) = G32(0x1015dc78) + 0x16a;
            Sim_1007df6c((const char *)(uintptr_t)G32(0x101c45c8), (const char *)(uintptr_t)G32(0x109b98c0), 0x1536,
                         100);
        } else if (G32(0x10179cd4) != 0) {
            G32(0x10151d50) = 0xfffffffeu;
            Sim_10044a69(0, 1);
            G32(0x1014cb1c) = 0;
            Sim_1003cbb3(0, 0);
            G32(0x1015f1d4) = G32(0x1015dc78) + 0x108;
            if (G32(0x1015ecd0) == 0) {
                G32(0x1015f1e4) = 1;
                G32(0x1015f1d0) = G32(0x1015dc78) + 0x16a;
                Sim_1007df6c((const char *)(uintptr_t)G32(0x101c45c8), (const char *)(uintptr_t)G32(0x109b98c0),
                             0x1536, 100);
            } else {
                G32(0x1015f1d0) = G32(0x1015dc78) + 0x43e;
            }
        }
    } else if (GS32(0x1015f1d0) > 0 && GS32(0x1015dc78) > GS32(0x1015f1d0)) {
        G32(0x1015f1e4) = 1;
        if (G32(0x1015ecd0) == 0 || G32(0x1015f1fc) != 0 || G32(0x1015ebf8) != 0) {
            W35_Quit();
        } else if (W35_CFG(8) == 0 || (W35_CFG(0xa) != 0 && W35_LIVES() == 0)) {
            /* game over: no continues */
            Sim_10077fff();
            Sim_10078177();
            G32(0x10144fa0) = 1;
#ifdef W35C_MUTATE /* the checker's negative check only: 1.0's clear of the settings byte */
            W35_CFG(8) = 0;
#endif
            G32(0x1015f1d0) = 0xffffffffu;
            Sim_1007df6c((const char *)(uintptr_t)G32(0x101c45c4), (const char *)(uintptr_t)G32(0x109b98c0), 0x58610,
                         100);
            W35_Stop();
            G32(0x1015f208) = 1;
            G32(0x1015ef10) = 1;
        } else {
            Sim_10077fff();
            Sim_10078177();
            G32(0x10144fa0) = 1;
            W35_Stop();
            G32(0x1015f1d0) = 0;
            G32(0x1015f208) = 2;
        }
    }
    if (G32(0x1015ebf8) != 0 && G32(0x1015f208) != 0) {
        G32(0x1015f1e4) = 0;
        G32(0x1015f1d0) = 0;
    }
    key = U16(GBYTES(0x1015fd4c), 0);
    if (G32(0x1015ec58) == 0)
        Sim_100488a3(key);
    if (GS32(0x10151d50) != -2 && W35_CALL_10048c84(key) != 0)
        return;
    key = (uint16_t)Sim_1005572c(key);
    if (key == 0)
        return;
    if (G32(0x1015f1e4) == 0) {
        Sim_1004954e(key);
    } else if (G32(0x1015ecd0) == 0 || G32(0x1015f1fc) != 0 || G32(0x1015ebf8) != 0) {
        if (key == 0x76)
            W35_Quit();
#ifdef W35P_MUTATE /* the checker's negative check only: 1.0's test, without the pending flag */
    } else if (W35_CFG(8) != 0) {
#else
    } else if (W35_CFG(8) != 0 && G32(0x1015ef10) == 0) {
#endif
        if (key == 0x4b) {
            Sim_10074bfa(5);
        } else if (key == 0x6b) {
            if (G32(0x1015ecd0) == 2 && W35_CFG(0xa) != 0) {
                if (G32(0x1015f200) == 0) {
                    Sim_10040ab8((int)W35_SELF());
                    G32(0x1015f200) = 1;
                }
            } else if (W35_CFG(0xa) != 0 && W35_LIVES() == 0) {
                G32(0x1015f1d0) = 0xffffffffu;
                W35_SPRINTF(text, (const char *)(uintptr_t)G32(0x101c47f0));
                Sim_1007df6c(text, SIM_PTR(const char, 0x1013b6d4), 0x58610, 100);    /* "BASE6X7" */
                G32(0x1015f208) = 1;
                G32(0x1015ef10) = 1;
            } else {
                /* continue: one life less, the player's gear back */
                if (W35_CFG(0xa) != 0)
                    W35_LIVES() = (uint8_t)(W35_LIVES() - 1);
                G32(0x1014cb1c) = 0;
                Sim_1003cbb3(0, 0);
#ifndef W35R_MUTATE /* the checker's negative check only: 1.0's, without the reset */
                Sim12_10089c60();
#endif
                W35_CALL_1009d200(W35_SELF(), 0, 0, 0);
                Sim_100448cd(0);
                *GBYTES(0x1015fdba) = 1;
                *GBYTES(0x1015fdbd) = 1;
                *GBYTES(0x1015fdd0) = 1;
                G32(0x1015d530) = 0;
                G32(0x1015f1d4) = 0;
                G32(0x1015f208) = 0;
                G32(0x1015f1d0) = 0;
                G32(0x1015f1e4) = 0;
                G32(0x1015f1fc) = 0;
                G32(0x1015ef10) = 0;
            }
        } else if (key == 0x76) {
            W35_Quit();
        }
    } else if (G32(0x1015ef10) != 0) {
        if (key == 0x4b)
            Sim_10074bfa(5);
        else if (key == 0x6b)
            Sim_10044a69(1, 0);
        else if (key == 0x76)
            W35_Quit();
    }
}
