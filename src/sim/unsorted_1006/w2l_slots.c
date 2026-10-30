#include "w2l.h"
#include "../util/access.h"
#include "../util/util.h"

#define SLOT(i) SIM_PTR(uint8_t, 0x10165be8u + (uint32_t)(i) * 0x18)
#define PAIR(i) SIM_PTR(uint8_t, 0x10144ed8u + (uint32_t)(i) * 8)
#define TABLE(i) (SIM_PTR(u32a, 0x109e8480)[(int32_t)(i)])

// FUNCTION: HGSIM 0x10062780
void __cdecl Sim_10062780(void)
{
    uint32_t object = PTRTAB(GS32(0x1017b8c8));
    /* Read up front like the original, even when no slot needs the value: a bad pointer faults here. */
    uint32_t kind = *(volatile u32a *)(BYTES(U32(object, 0x28)) + 0x613);
    int i;

    for (i = 0; i < 10; i++) {
        uint8_t *slot = SLOT(i);
        uint8_t *pair = PAIR(i);

        if (U32(slot, 0x10) == 0) {
            if (U32(slot, 0x14) != 0) {
                U32(slot, 0) = (uint32_t)(Sim_1009c929(3) + 1) * 0x5a;
                U32(slot, 0x10) = 1;
                TABLE(U32(pair, 0)) = TABLE(U32(pair, 4) + U32(slot, 4));
                U32(slot, 0x14) = 0;
            }
        } else {
            U32(slot, 0) = U32(slot, 0) - G32(0x1015dc80);
            if (S32(slot, 0) <= 0) {
                U32(slot, 0) = 0;
                U32(slot, 0x10) = 0;
                TABLE(U32(pair, 0)) = (uint32_t)(uintptr_t)SIM_PTR(
                    uint8_t, 0x101362e0u + (kind + G32(0x101c64f0) * 2u) * 0x770u + U32(pair, 0) * 16u);
                U32(slot, 0x14) = 0;
            } else {
                S32(slot, 4) = (int32_t)(U32(slot, 4) + 1u);
                S32(slot, 4) = S32(slot, 4) % 3;
                TABLE(U32(pair, 0)) = TABLE(U32(pair, 4) + U32(slot, 4));
            }
        }
    }
}
