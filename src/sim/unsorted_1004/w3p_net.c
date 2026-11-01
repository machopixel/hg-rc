#include "w3p.h"
#include "fns.h"
#include "w2h.h"
#include "w2i.h"
#include "../render/w2n.h"
#include "../unsorted_1003/fns.h"
#include "../unsorted_1007/w2f.h"
#include "../unsorted_1007/w3p.h"
#include "../unsorted_100f/w2e.h"
#include "../util/access.h"
#include "../util/w3p.h"

#define ROOT G32(0x1015d004)
#define SELF G32(0x1017b8c8)
#define BUF(var) BYTES(G32(var))
#define LIST_COUNT S32(ROOT, 0x12)
#define LIST_ID(i) U16(BYTES(U32(ROOT, 0x126)), (uint32_t)(i) * 0x1e + 0x1a)
#define LIST_KEY(i) U16(BYTES(U32(ROOT, 0x16)), (uint32_t)(i) * 2)
#define SESSION G32(0x1015ecd4)
#define WORD_ACAC (*SIM_PTR(u16a, 0x1013acac))
#define WORD_ECD8 (*SIM_PTR(u16a, 0x1015ecd8))
/* 1.2's kill table (0x1028cc2c): 10 x 10 words, a row of 0x14 per killer, a column per victim (round 33) */
#define KILLS(row, col) (*(u16a *)(g_w34Kills + (uint32_t)(row) * 0x14u + (uint32_t)(col) * 2u))

/* Round 33, 1.2's 10-player rating: the kill table (1.2 0x1028cc2c; 1.0's 8 x 8 at 0x1016552c, row 0x10), the
   ratings (1.2 0x107caa60; 1.0's 8 at 0x1015d0f0), the receive guard (1.2 0x101314c0, set by the message loop from
   the game-over flag) and the buffer of 1.2's new message "AK" (1.2's pointer 0x101314e8, one more 0x100 block) */
uint8_t g_w34Kills[10 * 0x14];
int16_t g_w34Rating[10];
uint32_t g_w34NetOver;
uint8_t g_w34AkBuf[0x100];
static const char kW34Ak[4] = "AK";

typedef int(__cdecl *CountFn)(uint32_t session);
typedef int(__cdecl *Fn3)(uint32_t session, uint32_t from, int value);
typedef int(__cdecl *Fn5)(uint32_t session, int a, const void *b, int c, int d);

static void Header(uint32_t bufVar, uint32_t headerAddr)
{
    Mem_Move(BUF(bufVar), GBYTES(headerAddr), 2);
}

// FUNCTION: HGSIM 0x10040376
void __cdecl Sim_10040376(int index)
{
    uint8_t *buf = BUF(0x1015ec90);
    uint8_t *entry;
    uint8_t *block;
    int i;

    Mem_Move(buf, GBYTES(0x1013ad68), 2);
    entry = BYTES(PTRTAB(index));
    if (entry == 0)
        return;
    block = BYTES(U32(entry, 0x28));
    buf[2] = (uint8_t)index;
    U16(buf, 3) = 0;
    for (i = 7; i >= 0; i--) {
        if (U32(block, 0xbf5 + (uint32_t)i * 4) != 0) {
            U16(buf, 3) = (uint16_t)(U16(buf, 3) | (uint16_t)(1u << i));
            U32(block, 0xbf5 + (uint32_t)i * 4) = 0;
        }
    }
    if (U16(buf, 3) != 0)
        Net_DpSend_1007e920(buf, 5, 0);
}

// FUNCTION: HGSIM 0x10040564
void __cdecl Sim_10040564(int index)
{
    uint8_t *entry = BYTES(PTRTAB(index));
    uint8_t *buf;

    if (entry == 0 || U32(entry, 0x14) != 3)
        return;
    buf = BUF(0x1015ec9c);
    buf[2] = (uint8_t)index;
    buf[3] = (uint8_t)U32(entry, 0xc);
    buf[4] = (uint8_t)SELF;
    U32(entry, 0xc) = SELF;
    Mem_Move(buf, GBYTES(0x1013ad6c), 2);
    Net_DpSend_1007e920(buf, 5, 1);
}

// FUNCTION: HGSIM 0x1004084a
void __cdecl Sim_1004084a(const uint8_t *msg)
{
    uint8_t *entry;

    if (SELF != msg[4])
        return;
    entry = BYTES(PTRTAB(msg[2]));
    if (entry == 0 || U32(entry, 0x14) != 3)
        return;
    Sim_10040721(entry, msg + 5);
    Sim_10040564((int)U32(entry, 4));
    U32(entry, 0x18) = 0;
}

// FUNCTION: HGSIM 0x100408bd
void __cdecl Sim_100408bd(int index)
{
    uint8_t *entry;
    uint8_t *buf;

    if (index < 0 || index >= GS32(0x1015dc64))
        return;
    entry = BYTES(PTRTAB(index));
    if (entry == 0 || U32(entry, 0x14) != 3 || U32(entry, 0xc) != SELF)
        return;
    buf = BUF(0x1015eca4);
    Mem_Move(buf, GBYTES(0x1013ad70), 2);
    buf[2] = (uint8_t)index;
    Sim_10040645(buf + 3, entry);
    Net_DpSend_1007e920(buf, 0x21, 0);
}

// FUNCTION: HGSIM 0x100409be
int __cdecl Sim_100409be(int kind, const uint16_t *words)
{
    uint8_t *buf = BUF(0x1015ecbc);

    Mem_Move(buf, GBYTES(0x1013ad74), 2);
    buf[2] = (uint8_t)kind;
    if (words == 0) {
        U16(buf, 3) = 0;
        U16(buf, 5) = 0;
    } else {
        U16(buf, 3) = words[0];
        U16(buf, 5) = words[1];
    }
    return Net_DpSend_1007e920(buf, 7, 1);
}

// FUNCTION: HGSIM 0x10040ab8
int __cdecl Sim_10040ab8(int kind)
{
    Header(0x1015eca8, 0x1013ad78);
    BUF(0x1015eca8)[2] = (uint8_t)kind;
    return Net_DpSend_1007e950(WORD_ECD8, BUF(0x1015eca8), 3, 1);
}

// FUNCTION: HGSIM 0x10040eb3
int __cdecl Sim_10040eb3(void)
{
    Header(0x1015ecc0, 0x1013ade0);
    U16(BUF(0x1015ecc0), 4) = (uint16_t)BYTES(ROOT)[0x138];
    G32(0x1013aca0) = (uint32_t)(int32_t)(int16_t)U16(BUF(0x1015ecc0), 4);
    U16(BUF(0x1015ecc0), 2) = (uint16_t)BYTES(ROOT)[0x137];
    G32(0x1013aca4) = (uint32_t)(int32_t)(int16_t)U16(BUF(0x1015ecc0), 2);
    return Net_DpSend_1007e920(BUF(0x1015ecc0), 6, 1);
}

// FUNCTION: HGSIM 0x10040f5f
void __cdecl Sim_10040f5f(const uint8_t *a, int w, int b, int c)
{
    int i;

    Header(0x1015ecac, 0x1013ade4);
    BUF(0x1015ecac)[2] = a[4];
    U16(BUF(0x1015ecac), 3) = (uint16_t)w;
    BUF(0x1015ecac)[5] = (uint8_t)b;
    U16(BUF(0x1015ecac), 6) = (uint16_t)c;
    if ((int16_t)w != 0x4200) {
        uint8_t *entry = BYTES(PTRTAB((uint32_t)(int32_t)(int16_t)w & 0xff));

        if (U32(entry, 0x14) == 3 && S32(entry, 0xc) >= 0)
            Net_DpSend_1007e950(LIST_KEY(Sim_1003dbc0(U32(entry, 0xc))), BUF(0x1015ecac), 8, 1);
        return;
    }
    for (i = 0; i < LIST_COUNT; i++) {
        uint32_t id = LIST_ID(i);

        if (PTRTAB(id) != 0 && U32(PTRTAB(id), 0x14) == 1 && Sim_10052be7((int)id) == 0 && id != SELF)
            Net_DpSend_1007e950(LIST_KEY(i), BUF(0x1015ecac), 8, 1);
    }
}

// FUNCTION: HGSIM 0x10041203
int __cdecl Sim_10041203(int kind, uint32_t value)
{
    Header(0x1015ecb4, 0x1013ade8);
    BUF(0x1015ecb4)[2] = (uint8_t)kind;
    U32(BUF(0x1015ecb4), 3) = value;
    return Net_DpSend_1007e920(BUF(0x1015ecb4), 7, 1);
}

// FUNCTION: HGSIM 0x100412cb
int __cdecl Sim_100412cb(int kind, int sub, uint32_t value)
{
    Header(0x1015ecb8, 0x1013adec);
    BUF(0x1015ecb8)[2] = (uint8_t)kind;
    BUF(0x1015ecb8)[3] = (uint8_t)sub;
    U32(BUF(0x1015ecb8), 4) = value;
    return Net_DpSend_1007e920(BUF(0x1015ecb8), 8, 1);
}

// FUNCTION: HGSIM 0x100413c7
// FUNCTION: HGSIM12 0x10035088
void __cdecl Sim_100413c7(void)
{
    int i;

    Header(0x1015ec98, 0x1013adf0);
    for (i = 0; i < 10 && PTRTAB(i) != 0; i++)
        BUF(0x1015ec98)[2 + i] = (uint8_t)KILLS(i, SELF);
    Net_DpSend_1007e920(BUF(0x1015ec98), 0xc, 1);
    Sim_10041444();
}

// FUNCTION: HGSIM 0x10041444
// FUNCTION: HGSIM12 0x10035105
void __cdecl Sim_10041444(void)
{
    int16_t before = g_w34Rating[SELF];
    int i, j;

    for (i = 0; i < 10; i++)
        g_w34Rating[i] = 0;
    for (i = 0; i < 10 && PTRTAB(i) != 0; i++) {
        for (j = 0; j < 10 && PTRTAB(j) != 0; j++) {
            if (U32(PTRTAB(i), 8) == U32(PTRTAB(j), 8))
                g_w34Rating[j] = (int16_t)(g_w34Rating[j] - (int16_t)KILLS(j, i));
            else
                g_w34Rating[j] = (int16_t)(g_w34Rating[j] + (int16_t)KILLS(j, i));
        }
    }
    if (before != g_w34Rating[SELF]) {
        int now = g_w34Rating[SELF];

        (*SIM_PTR(Fn3, 0x1015eb20))(SESSION, ((uint32_t)now & 0xffff0000u) | WORD_ACAC, now);
    }
}

// FUNCTION: HGSIM 0x1004158c
// FUNCTION: HGSIM12 0x1003524d
void __cdecl Sim_1004158c(const uint8_t *msg, int column)
{
    int i;

    if (g_w34NetOver != 0)
        return;
    for (i = 0; i < 10 && PTRTAB(i) != 0; i++)
        KILLS(i, column) = msg[2 + i];
    Sim_10041444();
    Sim_1007b02d();
}

/* 1.2's new message "AK": the kills of the unit `unit` (a kind-3 unit of the local player) by everyone */
// FUNCTION: HGSIM12 0x100352b7
void __cdecl Sim12_100352b7(uint8_t killer, int unit)
{
    int i;

    Mem_Move(g_w34AkBuf, kW34Ak, 2);
    g_w34AkBuf[2] = killer;
    g_w34AkBuf[3] = (uint8_t)unit;
    for (i = 0; i < 10 && PTRTAB(i) != 0; i++)
        g_w34AkBuf[4 + i] = (uint8_t)KILLS(i, unit);
    Net_DpSend_1007e920(g_w34AkBuf, 0xe, 1);
    Sim_10041444();
}

/* its receive: the column of the unit named in the message */
// FUNCTION: HGSIM12 0x10035348
void __cdecl Sim12_10035348(const uint8_t *msg)
{
    uint8_t unit;
    int i;

    if (g_w34NetOver != 0)
        return;
    unit = msg[3];
    for (i = 0; i < 10 && PTRTAB(i) != 0; i++)
        KILLS(i, unit) = msg[4 + i];
    Sim_10041444();
    Sim_1007b02d();
}

// FUNCTION: HGSIM 0x100415eb
int __cdecl Sim_100415eb(int id, float x, float y, float z)
{
    uint8_t *buf = BUF(0x1015d084);
    uint8_t *block;

    Mem_Move(buf, GBYTES(0x1013adf4), 2);
    U32(BUF(0x1015d084), 2) = (uint32_t)Render_Ftol((double)-x * (double)GF32(0x10111508) + (double)GF32(0x1011150c));
    U32(BUF(0x1015d084), 6) = (uint32_t)Render_Ftol((double)-y * (double)GF32(0x10111508) + (double)GF32(0x1011150c));
    U32(BUF(0x1015d084), 0xa) = (uint32_t)Render_Ftol((double)-z * (double)GF32(0x10111508) + (double)GF32(0x1011150c));
    block = BYTES(U32(PTRTAB(SELF), 0x28));
    U32(BUF(0x1015d084), 0xe) = (uint32_t)Render_Ftol((double)F32(block, 0x554) * (double)GF32(0x10111508) + (double)GF32(0x1011150c));
    U32(BUF(0x1015d084), 0x12) = (uint32_t)Render_Ftol((double)F32(block, 0x558) * (double)GF32(0x10111508) + (double)GF32(0x1011150c));
    U32(BUF(0x1015d084), 0x16) = (uint32_t)Render_Ftol((double)F32(block, 0x55c) * (double)GF32(0x10111508) + (double)GF32(0x1011150c));
    return Net_DpSend_1007e950(LIST_KEY(Sim_1003dbc0((uint32_t)id)), BUF(0x1015d084), 0x1a, 0);
}

// FUNCTION: HGSIM 0x1004185b
int __cdecl Sim_1004185b(void)
{
    Header(0x1015ec94, 0x1013ae04);
    return Net_DpSend_1007e920(BUF(0x1015ec94), 2, 0);
}

// FUNCTION: HGSIM 0x10041888
int __cdecl Sim_10041888(void)
{
    Header(0x1015ec94, 0x1013ae08);
    if (G32(0x1015ecd0) == 1) {
        *SIM_PTR(u16a, 0x1015ec7c) = 1;
        return Net_DpSend_1007e920(BUF(0x1015ec94), 2, 0);
    }
    return Net_DpSend_1007e950(WORD_ECD8, BUF(0x1015ec94), 2, 0);
}

// FUNCTION: HGSIM 0x100418e3
void __cdecl Sim_100418e3(int unused, int who)
{
    (void)unused;
    if (SELF == (uint32_t)who)
        return;
    if (G32(0x1015ecd0) == 1) {
        int n;

        GBYTES(0x1015d090)[who] = 1;
        n = (*SIM_PTR(CountFn, 0x1015ead8))(SESSION) - 1;
        while (n >= 0 && (int8_t)GBYTES(0x1015d090)[LIST_ID(n)] == 1)
            n--;
        if (n == -1)
            Sim_10041888();
    } else if ((uint32_t)who == Sim_1003db56((int16_t)WORD_ECD8)) {
        G32(0x1017b94c) |= 1;
        *SIM_PTR(u16a, 0x1015ec7c) = 1;
    }
}

// FUNCTION: HGSIM 0x10041997
int __cdecl Sim_10041997(void)
{
    Header(0x1015ec94, 0x1013ae0c);
    if (G32(0x1015ecd0) == 1)
        return Net_DpSend_1007e920(BUF(0x1015ec94), 2, 0);
    return Net_DpSend_1007e950(WORD_ECD8, BUF(0x1015ec94), 2, 0);
}

// FUNCTION: HGSIM 0x100419e9
int __cdecl Sim_100419e9(void)
{
    Header(0x1015ec94, 0x1013ae10);
    return Net_DpSend_1007e920(BUF(0x1015ec94), 2, 1);
}

// FUNCTION: HGSIM 0x10041a47
int __cdecl Sim_10041a47(void)
{
    uint8_t acc = 0;
    int bits = 0;
    int i;
    uint8_t *out = BUF(0x1015ecc4) + 2;

    G32(0x1015d08c) = 0x27;
    Mem_Move(BUF(0x1015ecc4), GBYTES(0x1013ae14), 2);
    U32(out, 0) = G32(0x1015ec68);
    out += 4;
    for (i = 0; i < 0xfe; i++) {
        acc = (uint8_t)(acc << 1);
        if (((int)(int16_t)*(u16a *)(GBYTES(0x109e9ba0) + (uint32_t)i * 0x50) & 4) != 0)
            acc |= 1;
        if (++bits == 8) {
            bits = 0;
            *out++ = acc;
            acc = 0;
        }
    }
    return Net_DpSend_1007e920(BUF(0x1015ecc4), (int)G32(0x1015d08c), 0);
}

// FUNCTION: HGSIM 0x10041c00
void __cdecl Sim_10041c00(void)
{
    Header(0x1015ec94, 0x1013ae18);
    if (G32(0x1015ec74) == 0) {
        G32(0x101655ac) = SELF;
        Net_DpSend_1007e920(BUF(0x1015ec94), 2, 1);
        G32(0x1015ec74) = 1;
    }
}

// FUNCTION: HGSIM 0x100420a0
int __cdecl Sim_100420a0(void)
{
    if (G32(0x1015ec58) != 0 && SESSION != 0 && G32(0x1015ece8) == 0) {
        G32(0x1015ece8) = 1;
        Header(0x1015ec94, 0x1013ae24);
        Net_DpSend_1007e920(BUF(0x1015ec94), 2, 1);
    }
    if (G32(0x1015eccc) != 1)
        return 1;
    G32(0x1015eccc) = 2;
    G32(0x1015ecd0) = 0;
    G32(0x1015dc84) = 0;
    return 1;
}

// FUNCTION: HGSIM 0x100421b0
void __cdecl Sim_100421b0(int word)
{
    *SIM_PTR(u16a, 0x1013acb0) = (uint16_t)word;
    WORD_ECD8 = WORD_ACAC;
    (*SIM_PTR(Fn5, 0x1015eae0))(SESSION, 0, SIM_PTR(void, 0x10001654), 0, 0);
    if ((*SIM_PTR(CountFn, 0x1015ead8))(SESSION) == 1) {
        G32(0x1015eccc) = 2;
        G32(0x1015ecd0) = 0;
        G32(0x1015dc84) = 0;
        *SIM_PTR(u16a, 0x1015ec7c) = 0;
    } else if ((uint32_t)WORD_ACAC == (uint32_t)WORD_ECD8) {
        G32(0x1015eccc) = 1;
        G32(0x1015ecd0) = 1;
        G32(0x1015dc84) = 0;
    } else {
        G32(0x1015eccc) = 1;
        G32(0x1015ecd0) = 2;
        G32(0x1015dc84) = 2;
    }
}
