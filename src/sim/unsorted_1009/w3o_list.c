#include "w3o.h"
#include "../fx/w32.h"
#include "../unsorted_1004/fns.h"
#include "w2l.h"
#include "../util/w3o.h"

#define RECORD(index) (0x10217c20u + (uint32_t)(index) * W32_REC_SIZE)

// FUNCTION: HGSIM 0x10090e23
void __cdecl Sim_10090e23(void)
{
    int removed = 0;
    uint32_t node;
    int i;

    if (G32(0x1017cc28) == 0 || G32(0x1015dc80) == 0 || G32(0x1017b92c) != 0)
        return;
    G32(0x10217c00) = 0;
    for (node = G32(0x1017cc28); node != 0; node = U32(node, 4)) {
        G32(0x10215be0u + G32(0x10217c00) * 4) = U32(node, 8);
        G32(0x10217c00) = G32(0x10217c00) + 1;
    }
    for (i = 0; i < GS32(0x10217c00); i++) {
        uint8_t *record = GBYTES(RECORD(G32(0x10215be0u + (uint32_t)i * 4)));
        uint32_t fn = U32(record, W32_REC_UPDATE);

        if (fn != 0 && ((int(__cdecl *)(void *))(uintptr_t)fn)(record) == 0) {
            removed++;
            Sim_10090513((int)U32(record, 8));
        }
    }
    G32(0x1017cc24) = G32(0x1017cc24) - (uint32_t)removed;
}

// FUNCTION: HGSIM 0x10090f17
void __cdecl Sim_10090f17(unsigned index)
{
    /* 1.2 (0x10079eee): first notes (Sim_100448ff() == 1) for the particle draws */
    if (W32_PARTICLES())
        g_w32_1014b58c = Sim_100448ff() == 1;
    if (G32(0x1017cc24) != 0 && index < G32(0x1017cc20) && (U16(GBYTES(RECORD(index)), 0x10) & 1) != 0) {
        uint8_t *record = GBYTES(RECORD(index));

        ((void(__cdecl *)(void *))(uintptr_t)U32(record, W32_REC_DRAW))(record);
        /* 1.2: a particle type with Lock set marks its system (bit 2 of +0x10) */
#ifdef W32F_MUTATE /* the checker's negative check only: Lock is not looked at */
        if (0)
#else
        if (W32_PARTICLES() && g_w32_1011e700[U32(record, 0x402)].lock != 0)
#endif
            U16(record, 0x10) = (uint16_t)(U16(record, 0x10) | 4);
    }
}
