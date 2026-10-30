#include "w3o.h"
#include "w2l.h"
#include "../util/w3o.h"

#define RECORD(index) (0x10217c20u + (uint32_t)(index) * 0x57e)

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
        uint32_t fn = U32(record, 0x556);

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
    if (G32(0x1017cc24) != 0 && index < G32(0x1017cc20) && (U16(GBYTES(RECORD(index)), 0x10) & 1) != 0) {
        uint8_t *record = GBYTES(RECORD(index));

        ((void(__cdecl *)(void *))(uintptr_t)U32(record, 0x55a))(record);
    }
}
