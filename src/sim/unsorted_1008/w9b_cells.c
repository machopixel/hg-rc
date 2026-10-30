#include "w9b.h"
#include "fns.h"
#include "w3s.h"
#include "../view/w2d.h"
#include "../view/w2l.h"
#include "../util/access.h"

static int W9b_IsKindC0(const uint8_t *node)
{
    return (U16(node, 2) & 0xf0) == 0xc0;
}

static void W9b_Relink(uint32_t first)
{
    uint8_t *node = BYTES(first);

    while (node != 0) {
        uint8_t *next = BYTES(U32(node, 8));

        if (W9b_IsKindC0(node)) {
            Sim_1008a40a(node);
            Sim_1008a37f(node);
        }
        node = next;
    }
}

// FUNCTION: HGSIM 0x1008a94d
void __cdecl Sim_1008a94d(int all)
{
    int col, row;

    if (all != 0) {
        uint8_t *node = BYTES(G32(0x1017b9e0));

        while (node != 0) {
            uint8_t *next = BYTES(U32(node, 8));

            if (W9b_IsKindC0(node)) {
                Sim_1008a46e(node);
                Sim_1008a220((uint16_t *)node);
            }
            node = next;
        }
        return;
    }
    if (G32(0x1017c8c4) != 0) {
        for (col = 0; col < 9; col++)
            for (row = 0; row < 9; row++)
                W9b_Relink(G32(0x1017ba30 + (uint32_t)col * 0x18c + (uint32_t)row * 0x2c));
    } else {
        W9b_Relink(G32(0x1017b990));
    }
}
