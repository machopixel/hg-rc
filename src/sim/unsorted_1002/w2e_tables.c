#include "../unsorted_1004/w3p.h"
#include "w2e.h"
#include "../unsorted_100f/w2e.h"
#include "../util/access.h"

// FUNCTION: HGSIM 0x10021ac0
void __cdecl Sim_10021ac0(void)
{
    int i;

    for (i = 0; i < 0x200; i++) {
        uint8_t *rec = GBYTES(0x1096fc80) + (uint32_t)i * 0x156u;

        U32(rec, 0) = 0xFFFFFFFFu;
        U32(rec, 0x3c) = 0;
        Sim_10021c3c(i);
    }
    for (i = 0; i < 0x100; i++) {
        uint8_t *rec = GBYTES(0x109a5a40) + (uint32_t)i * 0x2cu;

        U32(rec, 0) = 0;
        U32(rec, 4) = 0;
        U32(rec, 0x1c) = 0xFFFFFFFFu;
        U32(rec, 0x24) = 0xFFFFFFFFu;
        U32(rec, 0x28) = 0xFFFFFFFFu;
        Sim_10021be0(i);
    }
    G32(0x1015dbe0) = 0;
    G32(0x1015dbdc) = G32(0x1015dbe0);
    G32(0x1015dbd8) = G32(0x1015dbdc);
    G32(0x1015dbe8) = 0;
    G32(0x1015dbec) = G32(0x1015dbe8);
    G32(0x1015dbe4) = G32(0x1015dbec);
    G32(0x1015dbf0) = 0;
    Mem_Fill(GBYTES(0x101653e0), 0, 0x1d4);
#ifndef W2ET_MUTATE /* the checker's negative check only: the kill table kept */
    Mem_Fill(g_w34Kills, 0, sizeof(g_w34Kills));   /* 1.2 (0x100186d0 / 0x1001cf11): its record holds the table */
#endif
}

// FUNCTION: HGSIM 0x10021be0
void __cdecl Sim_10021be0(int index)
{
    uint8_t *rec = GBYTES(0x109a5a40) + (uint32_t)index * 0x2cu;

    U32(rec, 0x10) = 0;
    U32(rec, 0xc) = 0;
    U32(rec, 8) = 0;
    U32(rec, 0x14) = 0;
    U32(rec, 0x18) = 0;
    U32(rec, 0x20) = 0;
    U32(rec, 4) = 0;
}

// FUNCTION: HGSIM 0x10021c3c
void __cdecl Sim_10021c3c(int index)
{
    uint8_t *rec = GBYTES(0x1096fc80) + (uint32_t)index * 0x156u;

    U32(rec, 0x20) = 0;
    U32(rec, 0x1c) = 0;
    U32(rec, 0x18) = 0;
    U32(rec, 0x2c) = 0;
    U32(rec, 0x28) = 0;
    U32(rec, 0x24) = 0;
    U32(rec, 0x34) = 0;
    U32(rec, 0x30) = 0;
    U32(rec, 0x38) = 0;
    U32(rec, 8) = 0xFFFFFFFFu;
    U32(rec, 0x40) = 0;
    U32(rec, 0x44) = 0;
    U32(rec, 0x48) = 0;
    U32(rec, 0x4c) = 0;
    U32(rec, 0x5c) = 0;
    U32(rec, 0x58) = 0;
    U32(rec, 0x60) = 0;
    U32(rec, 0x64) = 0xbf800000u;
    U32(rec, 4) = 0;
    U32(rec, 0x68) = 0xFFFFFFFFu;
    U32(rec, 0x6c) = 0xFFFFFFFFu;
    U32(rec, 0x70) = 0;
    U32(rec, 0x74) = 0;
    U32(rec, 0x78) = 0;
    U16(rec, 0x7c) = 0;
    U32(rec, 0x7e) = 0;
    U32(rec, 0x152) = 0;
}

// FUNCTION: HGSIM 0x10025d93
void __cdecl Sim_10025d93(void)
{
    Mem_Fill(GBYTES(0x101653e0), 0, 0x1d4);
#ifndef W2ET_MUTATE /* the checker's negative check only: the kill table kept */
    Mem_Fill(g_w34Kills, 0, sizeof(g_w34Kills));   /* 1.2 (0x100186d0 / 0x1001cf11): its record holds the table */
#endif
}

// FUNCTION: HGSIM 0x10028571
void *__cdecl Sim_10028571(void *dst, const void *src, uint32_t n)
{
    return Mem_Move(dst, src, n);
}

// FUNCTION: HGSIM 0x1002858a
void *__cdecl Sim_1002858a(void *dst, int c, uint32_t n)
{
    return Mem_Fill(dst, c, n);
}
